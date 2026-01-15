#include "audio/Engine.hpp"
#include "StackTrace.hpp"
#include "audio/Constants.hpp"
#include "utils/string/SmallString.hpp"

namespace audio
{

auto AudioEngine::enumeratePlaybackDevices() -> Devices
{
  RECORD_FUNC_TO_BACKTRACE("AudioEngine::enumeratePlaybackDevices");

  Devices devices;
  devices.reserve(16);

  devices.push_back({"default", "Default Audio Device", -1, -1, true});

  int cardIdx = -1;
  while (snd_card_next(&cardIdx) == 0 && cardIdx >= 0)
  {
    char* cardName     = nullptr;
    char* cardLongName = nullptr;

    if (snd_card_get_name(cardIdx, &cardName) != 0)
      continue;

    snd_card_get_longname(cardIdx, &cardLongName);

    snd_ctl_t* ctl = nullptr;

    utils::string::SmallString ctlName;
    ctlName += "hw:";
    ctlName += cardIdx;

    if (snd_ctl_open(&ctl, ctlName.c_str(), 0) == 0)
    {
      int devIdx = -1;
      while (snd_ctl_pcm_next_device(ctl, &devIdx) == 0 && devIdx >= 0)
      {
        snd_pcm_info_t* pcmInfo;
        snd_pcm_info_alloca(&pcmInfo);

        snd_pcm_info_set_device(pcmInfo, devIdx);
        snd_pcm_info_set_subdevice(pcmInfo, 0);
        snd_pcm_info_set_stream(pcmInfo, SND_PCM_STREAM_PLAYBACK);

        if (snd_ctl_pcm_info(ctl, pcmInfo) < 0)
          continue;

        // Build deviceName once
        DeviceName deviceName("hw:");
        deviceName += cardIdx + ',' + devIdx;

        cstr pcmName = snd_pcm_info_get_name(pcmInfo);
        cstr pcmId   = snd_pcm_info_get_id(pcmInfo);

        // Build description efficiently
        utils::string::SmallString desc;

        if (cardLongName && pcmName)
        {
          desc += cardLongName;
          desc += " - ";
          desc += pcmName;

          if (pcmId && *pcmId)
          {
            desc += " (";
            desc += pcmId;
            desc += ")";
          }
        }
        else if (pcmName)
        {
          desc += cardName;
          desc += " - ";
          desc += pcmName;
        }
        else
        {
          desc += cardName;
          desc += " Device ";
          desc += devIdx;
        }

        desc += " [";
        desc += deviceName;
        desc += "]";

        devices.push_back(
          {std::move(deviceName.c_str()), std::move(desc.c_str()), cardIdx, devIdx, false});
      }

      snd_ctl_close(ctl);
    }

    free(cardName);
    free(cardLongName);
  }

  return devices;
}

void AudioEngine::initEngineForDevice(const DeviceName& deviceName)
{
  RECORD_FUNC_TO_BACKTRACE("AudioEngine::initEngineForDevice");

  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_pcmData)
  {
    shutdownAlsa();
  }

  m_currentDevice         = deviceName;
  m_backendInfo.dev.name  = deviceName;
  m_backendInfo.isActive  = false;
  m_backendInfo.isPlaying = false;
  m_backendInfo.isPaused  = false;
  m_backendInfo.xruns     = 0;
  m_backendInfo.writes    = 0;

  initAlsa(deviceName);
}

auto AudioEngine::loadSound(const Path& path) -> bool
{
  RECORD_FUNC_TO_BACKTRACE("AudioEngine::loadSound");

  std::lock_guard<std::mutex> lock(m_mutex);

  try
  {
    auto s = std::make_unique<Sound>();

    AVFormatContext* rawFmt = nullptr;
    if (avformat_open_input(&rawFmt, path.c_str(), nullptr, nullptr) < 0)
      return false;
    s->fmt.reset(rawFmt);

    if (avformat_find_stream_info(s->fmt.get(), nullptr) < 0)
      return false;

    s->streamIndex = av_find_best_stream(s->fmt.get(), AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (s->streamIndex < 0)
      return false;

    s->stream = s->fmt->streams[s->streamIndex];

    const AVCodec* codec = avcodec_find_decoder(s->stream->codecpar->codec_id);
    if (!codec)
      return false;

    m_backendInfo.codecName     = codec->name ? codec->name : "<unknown-codec-name>";
    m_backendInfo.codecLongName = codec->long_name ? codec->long_name : "<unknown-codec-longName>";

    AVCodecContext* rawDec = avcodec_alloc_context3(codec);
    if (!rawDec)
      return false;
    s->dec.reset(rawDec);

    if (avcodec_parameters_to_context(s->dec.get(), s->stream->codecpar) < 0 ||
        avcodec_open2(s->dec.get(), codec, nullptr) < 0)
      return false;

    // -------------------------------------------------------
    // SOURCE FORMAT (immutable, from file)
    // -------------------------------------------------------
    s->source.sampleRate    = s->dec->sample_rate;
    s->source.channels      = s->dec->ch_layout.nb_channels;
    s->source.sampleFmt     = s->dec->sample_fmt;
    s->source.sampleFmtName = av_get_sample_fmt_name(s->dec->sample_fmt)
                                ? av_get_sample_fmt_name(s->dec->sample_fmt)
                                : "unknown";

    av_channel_layout_copy(&s->source.channelLayout, &s->dec->ch_layout);

    // -------------------------------------------------------
    // TARGET FORMAT (engine / backend)
    // -------------------------------------------------------
    s->target.sampleRate    = m_backendInfo.sampleRate;
    s->target.channels      = m_backendInfo.channels;
    s->target.sampleFmt     = AV_SAMPLE_FMT_FLT;
    s->target.sampleFmtName = "float";

    av_channel_layout_default(&s->target.channelLayout, s->target.channels);

    // -------------------------------------------------------
    // Duration + encoder padding
    // -------------------------------------------------------
    if (auto* e = av_dict_get(s->fmt->metadata, "encoder_delay", nullptr, 0))
      s->startSkip = std::stoll(e->value);

    if (auto* e = av_dict_get(s->fmt->metadata, "encoder_padding", nullptr, 0))
      s->endSkip = std::stoll(e->value);

    s->durationFrames = (s->stream->duration * s->source.sampleRate * s->stream->time_base.num) /
                          s->stream->time_base.den -
                        s->startSkip - s->endSkip;

    // -------------------------------------------------------
    // Resampler: SOURCE → TARGET
    // -------------------------------------------------------
    SwrContext* rawSwr = nullptr;
    if (swr_alloc_set_opts2(&rawSwr, &s->target.channelLayout, s->target.sampleFmt,
                            s->target.sampleRate, &s->source.channelLayout, s->source.sampleFmt,
                            s->source.sampleRate, 0, nullptr) < 0)
      return false;

    s->swr.reset(rawSwr);

    if (swr_init(s->swr.get()) < 0)
      return false;

    // -------------------------------------------------------
    // Buffers use TARGET format (what we actually play)
    // -------------------------------------------------------
    s->sampleRate = s->target.sampleRate;
    s->channels   = s->target.channels;

    s->initializeBuffers();

    m_sound = std::move(s);
    return true;
  }
  catch (...)
  {
    return false;
  }
}

auto AudioEngine::getSound() -> Sound& { return *m_sound; }

auto AudioEngine::getSound() const -> const Sound& { return *m_sound; }

void AudioEngine::play()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_playbackState == PlaybackState::Playing)
    return;

  if (m_playbackState == PlaybackState::Paused && m_pcmData)
    snd_pcm_prepare(m_pcmData);

  m_playbackState         = PlaybackState::Playing;
  m_backendInfo.isPlaying = true;
  m_backendInfo.isPaused  = false;
  m_backendInfo.isActive  = true;

  startThread();
}

void AudioEngine::pause()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_playbackState != PlaybackState::Playing)
    return;

  m_playbackState         = PlaybackState::Paused;
  m_backendInfo.isPaused  = true;
  m_backendInfo.isPlaying = false;

  if (m_pcmData)
  {
    snd_pcm_drop(m_pcmData);
    snd_pcm_prepare(m_pcmData);
  }
}

void AudioEngine::stop()
{
  RECORD_FUNC_TO_BACKTRACE("AudioEngine::stop");

  std::unique_lock<std::mutex> lock(m_mutex);
  m_playbackState         = PlaybackState::Stopped;
  m_backendInfo.isActive  = false;
  m_backendInfo.isPlaying = false;
  m_backendInfo.isPaused  = false;
  m_isRunning             = false;
  lock.unlock();

  if (m_audioThread.joinable())
    m_audioThread.join();

  lock.lock();
  if (m_pcmData)
  {
    snd_pcm_drop(m_pcmData);
    snd_pcm_prepare(m_pcmData);
  }
}

auto AudioEngine::getPlaybackTime() const -> std::optional<std::pair<double, double>>
{
  if (!m_sound)
    return std::nullopt;

  auto& s = *m_sound;

  return std::pair{double(s.cursorFrames.load()) / s.sampleRate,
                   double(s.durationFrames) / s.sampleRate};
}

// seek

void AudioEngine::seekToAbsolute(double seconds)
{
  RECORD_FUNC_TO_BACKTRACE("AudioEngine::seekAbsolute");

  if (!m_sound)
    return;

  auto& s = *m_sound;

  if (s.sampleRate <= 0)
    return;

  // Clamp time
  double durationSec = double(s.durationFrames) / double(s.sampleRate);

  seconds = std::clamp(seconds, 0.0, durationSec);

  // Convert to frame index (including encoder delay)
  i64 targetFrame = static_cast<i64>(seconds * s.sampleRate) + s.startSkip;

  // Clamp frame bounds
  targetFrame = std::clamp(targetFrame, i64(s.startSkip), i64(s.startSkip + s.durationFrames));

  // Signal decode thread
  s.seekTargetFrame.store(targetFrame, std::memory_order_release);
  s.seekPending.store(true, std::memory_order_release);
}

void AudioEngine::seekForward(double sec)
{
  auto time = getPlaybackTime();
  if (!time)
    return;

  auto [p, l] = *time;
  seekToAbsolute(std::min(p + sec, l));
}

void AudioEngine::seekBackward(double sec)
{
  auto time = getPlaybackTime();
  if (!time)
    return;

  auto [p, _] = *time;
  seekToAbsolute(std::max(0.0, p - sec));
}

// private methods

void AudioEngine::initAlsa(const DeviceName& deviceName)
{
  int err;
  if ((err = snd_pcm_open(&m_pcmData, deviceName.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    throw std::runtime_error(snd_strerror(err));

  std::array<snd_pcm_format_t, 3> formats = {SND_PCM_FORMAT_FLOAT_LE, SND_PCM_FORMAT_S32_LE,
                                             SND_PCM_FORMAT_S16_LE};

  for (auto f : formats)
  {
    err = snd_pcm_set_params(m_pcmData, f, SND_PCM_ACCESS_RW_INTERLEAVED, m_backendInfo.channels,
                             m_backendInfo.sampleRate, 1, 50000);
    if (err >= 0)
    {
      m_backendInfo.pcmFormat     = f;
      m_backendInfo.pcmFormatName = snd_pcm_format_name(f);
      break;
    }
  }

  snd_pcm_hw_params_t* hw;
  snd_pcm_hw_params_alloca(&hw);
  snd_pcm_hw_params_current(m_pcmData, hw);

  snd_pcm_hw_params_get_channels(hw, &m_backendInfo.channels);
  snd_pcm_hw_params_get_rate(hw, &m_backendInfo.sampleRate, nullptr);
  snd_pcm_hw_params_get_period_size(hw, &m_backendInfo.periodSize, nullptr);
  snd_pcm_hw_params_get_buffer_size(hw, &m_backendInfo.bufferSize);

  m_backendInfo.latencyMs = double(m_backendInfo.bufferSize) / m_backendInfo.sampleRate * 1000.0;
}

void AudioEngine::shutdownAlsa()
{
  if (m_pcmData)
  {
    snd_pcm_drain(m_pcmData);
    snd_pcm_close(m_pcmData);
    m_pcmData = nullptr;
  }
}

void AudioEngine::decodeAndPlay()
{

  const auto BufSize = constants::FramesPerBuffer * constants::MaxChannels;

  static thread_local std::vector<float> playbackBuffer(BufSize);
  static thread_local std::vector<i16>   playbackBuffer16(BufSize);
  static thread_local std::vector<i32>   playbackBuffer32(BufSize);

  auto& s = *m_sound;

  if (s.seekPending.exchange(false))
  {
    i64 frame = s.seekTargetFrame.load();
    av_seek_frame(s.fmt.get(), s.streamIndex,
                  av_rescale_q(frame, {1, s.sampleRate}, s.stream->time_base),
                  AVSEEK_FLAG_BACKWARD);

    avcodec_flush_buffers(s.dec.get());
    s.ring->clear();
    s.cursorFrames.store(frame - s.startSkip);
    s.eof = false;
  }

  if (s.eof)
    return;

  // Decode more data if ring buffer needs more
  // Check for samples: 512 frames * channels
  const size_t samplesNeeded = constants::FramesPerBuffer * s.channels;
  while (s.ring->available() < samplesNeeded && s.ring->space() > 0 && !s.eof)
  {
    decodeStep(s);
  }

  if (s.ring->available() >= samplesNeeded)
  {
    // Read samples from ring buffer
    size_t samplesRead = s.ring->read(playbackBuffer.data(), samplesNeeded);
    size_t framesRead  = samplesRead / s.channels;

    if (framesRead > 0)
    {
      float vol = m_volume.load();

      if (m_backendInfo.pcmFormat == SND_PCM_FORMAT_FLOAT_LE)
      {
        for (size_t i = 0; i < samplesRead; ++i)
          playbackBuffer[i] *= vol;

        int r = snd_pcm_writei(m_pcmData, playbackBuffer.data(), framesRead);
        if (r == -EPIPE)
        {
          snd_pcm_prepare(m_pcmData);
        }
        else if (r < 0)
        {
          snd_pcm_recover(m_pcmData, r, 0);
        }
      }
      else if (m_backendInfo.pcmFormat == SND_PCM_FORMAT_S16_LE)
      {
        for (size_t i = 0; i < samplesRead; ++i)
        {
          float sample =
            std::clamp(playbackBuffer[i] * vol, constants::FloatMin, constants::FloatMax);
          playbackBuffer16[i] = static_cast<i16>(sample * constants::S16MaxFloat);
        }

        int r = snd_pcm_writei(m_pcmData, playbackBuffer16.data(), framesRead);
        if (r == -EPIPE)
        {
          snd_pcm_prepare(m_pcmData);
        }
        else if (r < 0)
        {
          snd_pcm_recover(m_pcmData, r, 0);
        }
      }
      else if (m_backendInfo.pcmFormat == SND_PCM_FORMAT_S32_LE)
      {
        for (size_t i = 0; i < samplesRead; ++i)
        {
          float sample =
            std::clamp(playbackBuffer[i] * vol, constants::FloatMin, constants::FloatMax);
          playbackBuffer32[i] = static_cast<i32>(sample * constants::S32MaxFloat);
        }

        int r = snd_pcm_writei(m_pcmData, playbackBuffer32.data(), framesRead);
        if (r == -EPIPE)
        {
          snd_pcm_prepare(m_pcmData);
        }
        else if (r < 0)
        {
          snd_pcm_recover(m_pcmData, r, 0);
        }
      }

      s.cursorFrames.fetch_add((i64)framesRead, std::memory_order_relaxed);
    }
  }
}

void AudioEngine::decodeStep(Sound& s)
{
  if (!s.frame)
  {
    s.eof = true;
    return;
  }

  // Read one packet
  const int rr = av_read_frame(s.fmt.get(), &s.pkt);
  if (rr < 0)
  {
    s.eof = true;
    return;
  }

  // Not our stream -> ignore quickly
  if (s.pkt.stream_index != s.streamIndex)
  {
    av_packet_unref(&s.pkt);
    return;
  }

  // Send packet to decoder
  int r = avcodec_send_packet(s.dec.get(), &s.pkt);
  av_packet_unref(&s.pkt);

  if (r < 0)
    return;

  while (true)
  {
    r = avcodec_receive_frame(s.dec.get(), s.frame.get());
    if (r == AVERROR(EAGAIN) || r == AVERROR_EOF)
      break;

    if (r < 0)
      break;

    // Compute required output frames
    const i64 delay     = swr_get_delay(s.swr.get(), s.source.sampleRate);
    const int outFrames = (int)av_rescale_rnd(delay + s.frame->nb_samples, s.target.sampleRate,
                                              s.source.sampleRate, AV_ROUND_UP);

    if (outFrames <= 0)
    {
      av_frame_unref(s.frame.get());
      continue;
    }

    const size_t totalSamples = (size_t)outFrames * (size_t)s.target.channels;

    // Not enough ring space -> stop decoding now (try later)
    if (s.ring->space() < totalSamples)
    {
      av_frame_unref(s.frame.get());
      break;
    }

    if (s.decodeBuffer.size() < totalSamples)
      s.decodeBuffer.resize(totalSamples);

    ui8* outData[1] = {reinterpret_cast<ui8*>(s.decodeBuffer.data())};

    const int framesConverted =
      swr_convert(s.swr.get(), outData, outFrames, (const ui8**)s.frame->data, s.frame->nb_samples);

    av_frame_unref(s.frame.get());

    if (framesConverted <= 0)
      continue;

    const size_t samplesToWrite = (size_t)framesConverted * (size_t)s.target.channels;

    // Safety: ring space was checked against totalSamples,
    // but samplesToWrite is usually <= totalSamples.
    s.ring->write(s.decodeBuffer.data(), samplesToWrite);
  }
}

} // namespace audio
