#include "audio/backend/alsa/Impl.hpp"
#include "Logger.hpp"
#include "StackTrace.hpp"
#include "audio/Constants.hpp"
#include "utils/string/SmallString.hpp"
#include <mutex>

namespace audio::backend
{

static auto alsaInfo(BackendInfo& info) -> AlsaBackendInfo&
{
  return std::get<AlsaBackendInfo>(info.specific);
}

auto AlsaBackend::enumerateDevices() -> Devices
{
  RECORD_FUNC_TO_BACKTRACE("AlsaBackend::enumeratePlaybackDevices");

  Devices devices;
  devices.reserve(16);

  devices.push_back({"default", "System Default Audio Device", -1, -1, true});

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

void AlsaBackend::switchDevice(const DeviceName& deviceName)
{
  RECORD_FUNC_TO_BACKTRACE("AlsaBackend::switchDeviceOutput");

  {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (deviceName == m_currentDevice)
      return;

    m_pendingDevice = deviceName;
    m_switchPending.store(true, std::memory_order_release);
  }
}

void AlsaBackend::switchAlsaDevice()
{
  RECORD_FUNC_TO_BACKTRACE("AlsaBackend::switchAlsaDevice");

  // Stop ALSA writes temporarily
  if (m_pcmData)
  {
    snd_pcm_drop(m_pcmData);
    snd_pcm_close(m_pcmData);
    m_pcmData = nullptr;
  }

  // Switch device
  m_currentDevice               = m_pendingDevice;
  m_backendInfo.common.dev.name = m_currentDevice;

  // Reinitialize ALSA
  initAlsa(m_currentDevice);

  // Prepare PCM for playback
  snd_pcm_prepare(m_pcmData);

  m_backendInfo.common.isActive = true;
}

void AlsaBackend::initForDevice(const DeviceName& deviceName)
{
  RECORD_FUNC_TO_BACKTRACE("AlsaBackend::initEngineForDevice");

  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_pcmData)
  {
    shutdownAlsa();
  }

  m_currentDevice                = deviceName;
  m_backendInfo.common.dev.name  = deviceName;
  m_backendInfo.common.isActive  = false;
  m_backendInfo.common.isPlaying = false;
  m_backendInfo.common.isPaused  = false;
  m_backendInfo.common.xruns     = 0;
  m_backendInfo.common.writes    = 0;

  initAlsa(deviceName);
}

auto AlsaBackend::prepareSound(const Path& path) -> std::shared_ptr<Sound>
{
  auto s = std::make_shared<Sound>();

  AVFormatContext* rawFmt = nullptr;
  if (avformat_open_input(&rawFmt, path.c_str(), nullptr, nullptr) < 0)
    return nullptr;
  s->fmt.reset(rawFmt);

  if (avformat_find_stream_info(s->fmt.get(), nullptr) < 0)
    return nullptr;

  s->streamIndex = av_find_best_stream(s->fmt.get(), AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
  if (s->streamIndex < 0)
    return nullptr;

  s->stream = s->fmt->streams[s->streamIndex];

  const AVCodec* codec = avcodec_find_decoder(s->stream->codecpar->codec_id);
  if (!codec)
    return nullptr;

  m_backendInfo.common.codecName = codec->name ? codec->name : "<unknown-codec-name>";
  m_backendInfo.common.codecLongName =
    codec->long_name ? codec->long_name : "<unknown-codec-longName>";

  AVCodecContext* rawDec = avcodec_alloc_context3(codec);
  if (!rawDec)
    return nullptr;
  s->dec.reset(rawDec);

  if (avcodec_parameters_to_context(s->dec.get(), s->stream->codecpar) < 0 ||
      avcodec_open2(s->dec.get(), codec, nullptr) < 0)
    return nullptr;

  // SOURCE format
  s->source.sampleRate    = s->dec->sample_rate;
  s->source.channels      = s->dec->ch_layout.nb_channels;
  s->source.sampleFmt     = s->dec->sample_fmt;
  s->source.sampleFmtName = av_get_sample_fmt_name(s->dec->sample_fmt)
                              ? av_get_sample_fmt_name(s->dec->sample_fmt)
                              : "<unknown-format-name>";

  av_channel_layout_copy(&s->source.channelLayout, &s->dec->ch_layout);

  // TARGET format (engine output)
  s->target.sampleRate    = m_backendInfo.common.sampleRate;
  s->target.channels      = m_backendInfo.common.channels;
  s->target.sampleFmt     = AV_SAMPLE_FMT_FLT;
  s->target.sampleFmtName = "float";

  av_channel_layout_default(&s->target.channelLayout, s->target.channels);

  // encoder delay/padding (gapless metadata)
  if (auto* e = av_dict_get(s->fmt->metadata, "encoder_delay", nullptr, 0))
    s->startSkip = std::stoll(e->value);
  if (auto* e = av_dict_get(s->fmt->metadata, "encoder_padding", nullptr, 0))
    s->endSkip = std::stoll(e->value);

  s->durationFrames = (s->stream->duration * s->source.sampleRate * s->stream->time_base.num) /
                        s->stream->time_base.den -
                      s->startSkip - s->endSkip;

  // Resampler
  SwrContext* rawSwr = nullptr;
  if (swr_alloc_set_opts2(&rawSwr, &s->target.channelLayout, s->target.sampleFmt,
                          s->target.sampleRate, &s->source.channelLayout, s->source.sampleFmt,
                          s->source.sampleRate, 0, nullptr) < 0)
    return nullptr;

  s->swr.reset(rawSwr);
  if (swr_init(s->swr.get()) < 0)
    return nullptr;

  // Playback format
  s->sampleRate = s->target.sampleRate;
  s->channels   = s->target.channels;

  s->initializeBuffers();

  return s;
}

auto AlsaBackend::load(const Path& path) -> bool
{
  RECORD_FUNC_TO_BACKTRACE("AlsaBackend::loadSound");

  std::lock_guard<std::mutex> lock(m_mutex);

  auto soundSharedPtr = prepareSound(path);
  if (!soundSharedPtr)
    return false;

  m_sound = std::move(soundSharedPtr);
  return true;
}

auto AlsaBackend::getSoundPtr() const -> std::shared_ptr<const Sound>
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_sound;
}

auto AlsaBackend::getSoundPtrMut() const -> std::shared_ptr<Sound>
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_sound;
}

// logic is identical to loadSound; just that nextSound is loaded into memory
// for gapless playback
auto AlsaBackend::queueNext(const Path& path) -> bool
{
  RECORD_FUNC_TO_BACKTRACE("AlsaBackend::queueNextSound");

  std::lock_guard<std::mutex> lock(m_mutex);

  auto s = prepareSound(path);
  if (!s)
    return false;

  m_nextSound = std::move(s);
  return true;
}

auto AlsaBackend::returnNextSoundIfQueued() -> std::shared_ptr<Sound>
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_nextSound)
    m_sound = std::move(m_nextSound);

  return m_sound;
}

void AlsaBackend::play()
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_playbackState == PlaybackState::Playing)
    return;

  if (m_playbackState == PlaybackState::Paused && m_pcmData)
    snd_pcm_prepare(m_pcmData);

  m_playbackState                = PlaybackState::Playing;
  m_backendInfo.common.isPlaying = true;
  m_backendInfo.common.isPaused  = false;
  m_backendInfo.common.isActive  = true;

  startThread();
}

void AlsaBackend::pause()
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_playbackState != PlaybackState::Playing)
    return;

  m_playbackState                = PlaybackState::Paused;
  m_backendInfo.common.isPaused  = true;
  m_backendInfo.common.isPlaying = false;

  if (m_pcmData)
  {
    snd_pcm_drop(m_pcmData);
    snd_pcm_prepare(m_pcmData);
  }
}

void AlsaBackend::stop()
{
  RECORD_FUNC_TO_BACKTRACE("AlsaBackend::stop");

  std::unique_lock<std::mutex> lock(m_mutex);

  m_playbackState                = PlaybackState::Stopped;
  m_backendInfo.common.isActive  = false;
  m_backendInfo.common.isPlaying = false;
  m_backendInfo.common.isPaused  = false;
  m_isRunning                    = false;
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

auto AlsaBackend::playbackTime() const -> std::optional<std::pair<double, double>>
{
  auto sound = getSoundPtr();
  if (!sound)
    return std::nullopt;
  auto& s = *sound;

  return std::pair{double(s.cursorFrames.load()) / s.sampleRate,
                   double(s.durationFrames) / s.sampleRate};
}

// seek

void AlsaBackend::seekAbsolute(double seconds)
{
  RECORD_FUNC_TO_BACKTRACE("AlsaBackend::seekAbsolute");

  auto sound = getSoundPtrMut();
  if (!sound)
    return;
  auto& s = *sound;

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

void AlsaBackend::seekForward(double sec)
{
  auto time = playbackTime();
  if (!time)
    return;

  auto [p, l] = *time;
  seekAbsolute(std::min(p + sec, l));
}

void AlsaBackend::seekBackward(double sec)
{
  auto time = playbackTime();
  if (!time)
    return;

  auto [p, _] = *time;
  seekAbsolute(std::max(0.0, p - sec));
}

// private methods

void AlsaBackend::initAlsa(const DeviceName& deviceName)
{
  m_backendInfo.specific.emplace<AlsaBackendInfo>();

  int err;
  if ((err = snd_pcm_open(&m_pcmData, deviceName.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    throw std::runtime_error(snd_strerror(err));

  std::array<snd_pcm_format_t, 3> formats = {SND_PCM_FORMAT_FLOAT_LE, SND_PCM_FORMAT_S32_LE,
                                             SND_PCM_FORMAT_S16_LE};

  for (auto f : formats)
  {
    err =
      snd_pcm_set_params(m_pcmData, f, SND_PCM_ACCESS_RW_INTERLEAVED, m_backendInfo.common.channels,
                         m_backendInfo.common.sampleRate, 1, 50000);
    if (err >= 0)
    {
      alsaInfo(m_backendInfo).pcmFormat     = f;
      alsaInfo(m_backendInfo).pcmFormatName = snd_pcm_format_name(f);
      break;
    }
  }

  const size_t maxSamples = constants::FramesPerBuffer * m_backendInfo.common.channels;
  m_playbackBuffer.resize(maxSamples);

  // worst-case scratch size (S32 is largest)
  m_scratchBuffer.resize(maxSamples * sizeof(i32));

  snd_pcm_hw_params_t* hw;
  snd_pcm_hw_params_alloca(&hw);
  snd_pcm_hw_params_current(m_pcmData, hw);

  snd_pcm_hw_params_get_channels(hw, &m_backendInfo.common.channels);
  snd_pcm_hw_params_get_rate(hw, &m_backendInfo.common.sampleRate, nullptr);
  snd_pcm_hw_params_get_period_size(hw, &alsaInfo(m_backendInfo).periodSize, nullptr);
  snd_pcm_hw_params_get_buffer_size(hw, &alsaInfo(m_backendInfo).bufferSize);

  m_backendInfo.common.latencyMs =
    double(alsaInfo(m_backendInfo).bufferSize) / m_backendInfo.common.sampleRate * 1000.0;
}

void AlsaBackend::shutdownAlsa()
{
  if (m_pcmData)
  {
    snd_pcm_drain(m_pcmData);
    snd_pcm_close(m_pcmData);
    m_pcmData = nullptr;
  }
}

void AlsaBackend::decodeAndPlay()
{
  // first check for current device to then play to sink
  if (m_switchPending.exchange(false, std::memory_order_acquire))
  {
    switchAlsaDevice();
  }

  std::shared_ptr<Sound> sound;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    sound = m_sound;
  }

  auto& s = *sound;

  const size_t samplesNeeded = constants::FramesPerBuffer * s.channels;

  if (m_playbackBuffer.size() < samplesNeeded)
    m_playbackBuffer.resize(samplesNeeded);

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

  if (s.eof.load(std::memory_order_acquire) && s.ring->available() == 0)
  {
    // emit track finished
    m_trackFinished.exchange(true, std::memory_order_release);
    LOG_TRACE("AlsaBackend: track finished");

    std::shared_ptr<Sound> next;

    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (m_nextSound)
      {
        next    = std::move(m_nextSound);
        m_sound = next;
      }
    }

    if (next)
    {
      // switch to local reference
      sound    = next;
      auto& ns = *sound;

      ns.cursorFrames.store(0, std::memory_order_relaxed);
      ns.seekPending.store(false, std::memory_order_relaxed);
      ns.eof.store(false, std::memory_order_relaxed);

      return; // next loop iteration decodes new sound
    }

    return;
  }

  while (s.ring->available() < samplesNeeded && s.ring->space() > 0 && !s.eof)
    decodeStep(s);

  if (s.ring->available() < samplesNeeded)
  {
    // If current song ended and we don't have enough audio left,
    // try switching to next sound instead of returning silence.
    if (s.eof)
    {
      // we dont need to track the ptr returned by this
      returnNextSoundIfQueued();
    }
    return;
  }

  size_t samplesRead = s.ring->read(m_playbackBuffer.data(), samplesNeeded);

  if (samplesRead == 0)
    return;

  {
    std::lock_guard<std::mutex> copyLock(m_copyMutex);

    const size_t want = (m_copySamples > 0) ? m_copySamples : samplesRead;
    const size_t n    = std::min(want, samplesRead);

    m_copyBuffer.resize(n);
    std::memcpy(m_copyBuffer.data(), m_playbackBuffer.data(), n * sizeof(float));

    m_copySeq.fetch_add(1, std::memory_order_release);
  }

  size_t framesRead = samplesRead / s.channels;

  if (framesRead == 0)
    return;

  float vol = m_volume.load();

  const auto pcmFormat = alsaInfo(m_backendInfo).pcmFormat;

  if (pcmFormat == SND_PCM_FORMAT_FLOAT_LE)
  {
    for (size_t i = 0; i < samplesRead; ++i)
      m_playbackBuffer[i] *= vol;

    int r = snd_pcm_writei(m_pcmData, m_playbackBuffer.data(), framesRead);
    m_backendInfo.common.writes++;
    if (r == -EPIPE)
    {
      m_backendInfo.common.xruns++;
      snd_pcm_prepare(m_pcmData);
    }
    else if (r < 0)
      snd_pcm_recover(m_pcmData, r, 0);
  }
  else if (pcmFormat == SND_PCM_FORMAT_S16_LE)
  {
    const size_t bytes = samplesRead * sizeof(i16);
    if (m_scratchBuffer.size() < bytes)
      m_scratchBuffer.resize(bytes);

    auto* out = reinterpret_cast<i16*>(m_scratchBuffer.data());

    for (size_t i = 0; i < samplesRead; ++i)
    {
      float sample =
        std::clamp(m_playbackBuffer[i] * vol, constants::FloatMin, constants::FloatMax);
      out[i] = static_cast<i16>(sample * constants::S16MaxFloat);
    }

    int r = snd_pcm_writei(m_pcmData, m_playbackBuffer.data(), framesRead);
    m_backendInfo.common.writes++;
    if (r == -EPIPE)
    {
      m_backendInfo.common.xruns++;
      snd_pcm_prepare(m_pcmData);
    }
    else if (r < 0)
      snd_pcm_recover(m_pcmData, r, 0);
  }
  else if (pcmFormat == SND_PCM_FORMAT_S32_LE)
  {
    const size_t bytes = samplesRead * sizeof(i32);
    if (m_scratchBuffer.size() < bytes)
      m_scratchBuffer.resize(bytes);

    auto* out = reinterpret_cast<i32*>(m_scratchBuffer.data());

    for (size_t i = 0; i < samplesRead; ++i)
    {
      float sample =
        std::clamp(m_playbackBuffer[i] * vol, constants::FloatMin, constants::FloatMax);
      out[i] = static_cast<i32>(sample * constants::S32MaxFloat);
    }

    int r = snd_pcm_writei(m_pcmData, m_playbackBuffer.data(), framesRead);
    m_backendInfo.common.writes++;
    if (r == -EPIPE)
    {
      m_backendInfo.common.xruns++;
      snd_pcm_prepare(m_pcmData);
    }
    else if (r < 0)
      snd_pcm_recover(m_pcmData, r, 0);
  }

  s.cursorFrames.fetch_add((i64)framesRead, std::memory_order_relaxed);
}

void AlsaBackend::decodeStep(Sound& s)
{
  if (!s.frame)
  {
    s.frame.reset(av_frame_alloc());
    if (!s.frame)
      throw std::runtime_error("av_frame_alloc failed");
  }

  const int rr = av_read_frame(s.fmt.get(), &s.pkt);

  if (rr == AVERROR_EOF)
  {
    av_packet_unref(&s.pkt);
    s.eof = true;
    return;
  }

  if (rr < 0)
  {
    av_packet_unref(&s.pkt);
    return;
  }

  if (s.pkt.stream_index != s.streamIndex)
  {
    av_packet_unref(&s.pkt);
    return;
  }

  int r = avcodec_send_packet(s.dec.get(), &s.pkt);

  av_packet_unref(&s.pkt);

  if (r < 0)
    return;

  while (true)
  {
    r = avcodec_receive_frame(s.dec.get(), s.frame.get());
    if (r == AVERROR(EAGAIN))
      break;

    if (r == AVERROR_EOF)
    {
      s.eof = true;
      break;
    }

    if (r < 0)
      break;

    const i64 delay     = swr_get_delay(s.swr.get(), s.source.sampleRate);
    const int outFrames = (int)av_rescale_rnd(delay + s.frame->nb_samples, s.target.sampleRate,
                                              s.source.sampleRate, AV_ROUND_UP);

    if (outFrames <= 0)
    {
      av_frame_unref(s.frame.get());
      continue;
    }

    const size_t totalSamples = (size_t)outFrames * (size_t)s.target.channels;

    if (s.ring->space() < totalSamples)
    {
      av_frame_unref(s.frame.get());
      break;
    }

    if (s.decodeBuffer.size() < totalSamples)
      s.decodeBuffer.resize(totalSamples);

    std::array<ui8*, 1> outData = {reinterpret_cast<ui8*>(s.decodeBuffer.data())};

    const int framesConverted = swr_convert(s.swr.get(), outData.data(), outFrames,
                                            (const ui8**)s.frame->data, s.frame->nb_samples);

    av_frame_unref(s.frame.get());

    if (framesConverted <= 0)
      continue;

    const size_t samplesToWrite = (size_t)framesConverted * (size_t)s.target.channels;
    s.ring->write(s.decodeBuffer.data(), samplesToWrite);
  }
}

auto AlsaBackend::copySequence() const noexcept -> ui64
{
  return m_copySeq.load(std::memory_order_acquire);
}

auto AlsaBackend::copyBufferSize() const noexcept -> size_t
{
  std::lock_guard<std::mutex> copyLock(m_copyMutex);
  return m_copyBuffer.size();
}

} // namespace audio::backend
