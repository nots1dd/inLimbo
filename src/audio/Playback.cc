#include "audio/Playback.hpp"
#include "Logger.hpp"
#include "StackTrace.hpp"

namespace audio
{

auto AudioEngine::enumeratePlaybackDevices() -> Devices {

  RECORD_FUNC_TO_BACKTRACE("AudioEngine::enumeratePlaybackDevices");

  Devices devices;
  
  devices.push_back({"default", "Default Audio Device", -1, -1, true});
  
  int card = -1;
  while (snd_card_next(&card) == 0 && card >= 0) {
      char* cardName = nullptr;
      char* cardLongName = nullptr;
      
      if (snd_card_get_name(card, &cardName) == 0) {
          snd_card_get_longname(card, &cardLongName);
          
          int dev = -1;
          snd_ctl_t* ctl = nullptr;
          char ctlName[32];
          snprintf(ctlName, sizeof(ctlName), "hw:%d", card);
          
          if (snd_ctl_open(&ctl, ctlName, 0) == 0) {
              while (snd_ctl_pcm_next_device(ctl, &dev) == 0 && dev >= 0) {
                  snd_pcm_info_t* pcmInfo;
                  snd_pcm_info_alloca(&pcmInfo);
                  snd_pcm_info_set_device(pcmInfo, dev);
                  snd_pcm_info_set_subdevice(pcmInfo, 0);
                  snd_pcm_info_set_stream(pcmInfo, SND_PCM_STREAM_PLAYBACK);
                  
                  if (snd_ctl_pcm_info(ctl, pcmInfo) >= 0) {
                      char deviceName[64];
                      snprintf(deviceName, sizeof(deviceName), "hw:%d,%d", card, dev);
                      
                      const char* pcmName = snd_pcm_info_get_name(pcmInfo);
                      const char* pcmId = snd_pcm_info_get_id(pcmInfo);
                      
                      std::string desc;
                      
                      if (cardLongName && pcmName) {
                          desc = std::string(cardLongName) + " - " + pcmName;
                          if (pcmId && strlen(pcmId) > 0) {
                              desc += " (" + std::string(pcmId) + ")";
                          }
                      } else if (pcmName) {
                          desc = std::string(cardName) + " - " + pcmName;
                      } else {
                          desc = std::string(cardName) + " Device " + std::to_string(dev);
                      }
                      
                      desc += " [" + std::string(deviceName) + "]";
                      
                      devices.push_back({deviceName, desc, card, dev, false});
                  }
              }
              snd_ctl_close(ctl);
          }
          
          free(cardName);
          if (cardLongName) free(cardLongName);
      }
  }
  
  return devices;
}

void AudioEngine::initEngineForDevice(const std::string& deviceName) {

  RECORD_FUNC_TO_BACKTRACE("AudioEngine::initEngineForDevice");

  std::lock_guard<std::mutex> lock(m_mutex);
  
  if (m_pcmData) {
      shutdownAlsa();
  }
  
  m_currentDevice = deviceName;
  initAlsa(deviceName);
}

auto AudioEngine::loadSound(const std::string& path) -> std::optional<size_t> {

  RECORD_FUNC_TO_BACKTRACE("AudioEngine::loadSound");

  std::lock_guard<std::mutex> lock(m_mutex);
  
  try {
      auto s = std::make_unique<Sound>();

      // Open input with RAII wrapper
      AVFormatContext* rawFmt = nullptr;
      if (avformat_open_input(&rawFmt, path.c_str(), nullptr, nullptr) < 0) {
          return std::nullopt;
      }
      s->fmt.reset(rawFmt);

      if (avformat_find_stream_info(s->fmt.get(), nullptr) < 0) {
          return std::nullopt;
      }

      s->streamIndex = av_find_best_stream(s->fmt.get(), AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
      if (s->streamIndex < 0) {
          return std::nullopt;
      }

      s->stream = s->fmt->streams[s->streamIndex];

      const AVCodec* codec = avcodec_find_decoder(s->stream->codecpar->codec_id);
      if (!codec) {
          return std::nullopt;
      }

      AVCodecContext* rawDec = avcodec_alloc_context3(codec);
      if (!rawDec) {
          return std::nullopt;
      }
      s->dec.reset(rawDec);

      if (avcodec_parameters_to_context(s->dec.get(), s->stream->codecpar) < 0 ||
          avcodec_open2(s->dec.get(), codec, nullptr) < 0) {
          return std::nullopt;
      }

      s->sampleRate = s->dec->sample_rate;
      s->channels   = 2;

      // Extract encoder delay/padding
      AVDictionaryEntry* e;
      if ((e = av_dict_get(s->fmt->metadata, "encoder_delay", nullptr, 0)))
          s->startSkip = std::stoll(e->value);
      if ((e = av_dict_get(s->fmt->metadata, "encoder_padding", nullptr, 0)))
          s->endSkip = std::stoll(e->value);

      s->durationFrames =
          (s->stream->duration * s->sampleRate * s->stream->time_base.num) / 
          s->stream->time_base.den - s->startSkip - s->endSkip;

      // Setup resampler with RAII
      AVChannelLayout outLayout, inLayout;
      av_channel_layout_default(&outLayout, 2);
      av_channel_layout_copy(&inLayout, &s->dec->ch_layout);

      SwrContext* rawSwr = nullptr;
      if (swr_alloc_set_opts2(
              &rawSwr,
              &outLayout, AV_SAMPLE_FMT_FLT, s->sampleRate,
              &inLayout, s->dec->sample_fmt, s->dec->sample_rate,
              0, nullptr) < 0) {
          av_channel_layout_uninit(&outLayout);
          av_channel_layout_uninit(&inLayout);
          return std::nullopt;
      }
      s->swr.reset(rawSwr);

      if (swr_init(s->swr.get()) < 0) {
          av_channel_layout_uninit(&outLayout);
          av_channel_layout_uninit(&inLayout);
          return std::nullopt;
      }

      av_channel_layout_uninit(&outLayout);
      av_channel_layout_uninit(&inLayout);

      // NOW initialize buffers based on actual audio parameters
      s->initializeBuffers();

      size_t index = m_sounds.size();
      m_sounds.push_back(std::move(s));
      return index;
      
  } catch (const std::exception& e) {
      return std::nullopt;
  }
}

void AudioEngine::play(size_t index) {
    if (index >= m_sounds.size()) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_playbackState == PlaybackState::Playing) return;

    if (m_playbackState == PlaybackState::Paused && m_pcmData) {
        snd_pcm_prepare(m_pcmData);
    }

    m_playbackState = PlaybackState::Playing;
    startThread();
}

void AudioEngine::pause(size_t index) {
    if (index >= m_sounds.size()) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_playbackState != PlaybackState::Playing) return;

    m_playbackState = PlaybackState::Paused;
    if (m_pcmData) {
        snd_pcm_drop(m_pcmData);
        snd_pcm_prepare(m_pcmData);
    }
}

void AudioEngine::stop(size_t index) {

    RECORD_FUNC_TO_BACKTRACE("AudioEngine::stop");

    std::unique_lock<std::mutex> lock(m_mutex);
    m_playbackState = PlaybackState::Stopped;
    m_isRunning = false;
    lock.unlock();

    if (m_audioThread.joinable())
        m_audioThread.join();

    lock.lock();
    if (m_pcmData) {
        snd_pcm_drop(m_pcmData);
        snd_pcm_prepare(m_pcmData);
    }
}

auto AudioEngine::getPlaybackTime(size_t i) const 
  -> std::optional<std::pair<double, double>> {
  if (i >= m_sounds.size()) return std::nullopt;
  
  auto& s = *m_sounds[i];
  return std::pair{
      double(s.cursorFrames.load()) / s.sampleRate,
      double(s.durationFrames) / s.sampleRate
  };
}

// seek

void AudioEngine::seekTo(double seconds, size_t i) {
    if (i >= m_sounds.size()) return;
    
    auto& s = *m_sounds[i];
    i64 frame = seconds * s.sampleRate + s.startSkip;
    s.seekTargetFrame.store(frame);
    s.seekPending.store(true);
}

void AudioEngine::seekForward(double sec, size_t i) {
    auto time = getPlaybackTime(i);
    if (!time) return;
    
    auto [p, l] = *time;
    seekTo(std::min(p + sec, l), i);
}

void AudioEngine::seekBackward(double sec, size_t i) {
    auto time = getPlaybackTime(i);
    if (!time) return;
    
    auto [p, _] = *time;
    seekTo(std::max(0.0, p - sec), i);
}

// private methods

void AudioEngine::initAlsa(const std::string& deviceName) {
    int err;
    if ((err = snd_pcm_open(&m_pcmData, deviceName.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        throw std::runtime_error("Cannot open audio device: " + 
                               std::string(snd_strerror(err)));
    }
    
    std::array<snd_pcm_format_t, 3> formatsToTry = {
        SND_PCM_FORMAT_FLOAT_LE,
        SND_PCM_FORMAT_S32_LE,
        SND_PCM_FORMAT_S16_LE
    };
    
    bool success = false;
    for (auto format : formatsToTry) {
        err = snd_pcm_set_params(
            m_pcmData,
            format,
            SND_PCM_ACCESS_RW_INTERLEAVED,
            2,
            m_rate,
            1,
            50000);
        
        if (err >= 0) {
            m_pcmFormat = format;
            success = true;
            break;
        }
    }
    
    if (!success) {
        snd_pcm_close(m_pcmData);
        m_pcmData = nullptr;
        throw std::runtime_error("Cannot set audio parameters with any supported format: " + 
                               std::string(snd_strerror(err)));
    }
}

void AudioEngine::shutdownAlsa() {
    if (m_pcmData) {
        snd_pcm_drain(m_pcmData);
        snd_pcm_close(m_pcmData);
        m_pcmData = nullptr;
    }
}

void AudioEngine::decodeAndPlay() {

    constexpr size_t FRAMES_PER_BUFFER = 512;
    constexpr size_t MAX_CHANNELS = 8; // Support up to 7.1 audio
    static thread_local std::vector<float> playbackBuffer(FRAMES_PER_BUFFER * MAX_CHANNELS);
    static thread_local std::vector<i16> playbackBuffer16(FRAMES_PER_BUFFER * MAX_CHANNELS);
    static thread_local std::vector<i32> playbackBuffer32(FRAMES_PER_BUFFER * MAX_CHANNELS);

    for (const auto& sp : m_sounds) {
        auto& s = *sp;

        if (s.seekPending.exchange(false)) {
            i64 frame = s.seekTargetFrame.load();
            av_seek_frame(
                s.fmt.get(), s.streamIndex,
                av_rescale_q(frame, {1, s.sampleRate}, s.stream->time_base),
                AVSEEK_FLAG_BACKWARD);

            avcodec_flush_buffers(s.dec.get());
            s.ring->clear();
            s.cursorFrames.store(frame - s.startSkip);
            s.eof = false;
        }

        if (s.eof) continue;

        // Decode more data if ring buffer needs more
        // Check for samples: 512 frames * channels
        const size_t samplesNeeded = FRAMES_PER_BUFFER * s.channels;
        while (s.ring->available() < samplesNeeded && s.ring->space() > 0 && !s.eof) {
            decodeStep(s);
        }

        if (s.ring->available() >= samplesNeeded) {
            // Read samples from ring buffer
            size_t samplesRead = s.ring->read(playbackBuffer.data(), samplesNeeded);
            size_t framesRead = samplesRead / s.channels;
            
            if (framesRead > 0) {
                float vol = m_volume.load();
                
                if (m_pcmFormat == SND_PCM_FORMAT_FLOAT_LE) {
                    for (size_t i = 0; i < samplesRead; ++i)
                        playbackBuffer[i] *= vol;
                    
                    int r = snd_pcm_writei(m_pcmData, playbackBuffer.data(), framesRead);
                    if (r == -EPIPE) {
                        snd_pcm_prepare(m_pcmData);
                    } else if (r < 0) {
                        snd_pcm_recover(m_pcmData, r, 0);
                    }
                } else if (m_pcmFormat == SND_PCM_FORMAT_S16_LE) {
                    for (size_t i = 0; i < samplesRead; ++i) {
                        float sample = std::clamp(playbackBuffer[i] * vol, -1.0f, 1.0f);
                        playbackBuffer16[i] = static_cast<i16>(sample * 32767.0f);
                    }
                    
                    int r = snd_pcm_writei(m_pcmData, playbackBuffer16.data(), framesRead);
                    if (r == -EPIPE) {
                        snd_pcm_prepare(m_pcmData);
                    } else if (r < 0) {
                        snd_pcm_recover(m_pcmData, r, 0);
                    }
                } else if (m_pcmFormat == SND_PCM_FORMAT_S32_LE) {
                    for (size_t i = 0; i < samplesRead; ++i) {
                        float sample = std::clamp(playbackBuffer[i] * vol, -1.0f, 1.0f);
                        playbackBuffer32[i] = static_cast<i32>(sample * 2147483647.0f);
                    }
                    
                    int r = snd_pcm_writei(m_pcmData, playbackBuffer32.data(), framesRead);
                    if (r == -EPIPE) {
                        snd_pcm_prepare(m_pcmData);
                    } else if (r < 0) {
                        snd_pcm_recover(m_pcmData, r, 0);
                    }
                }

                s.cursorFrames += framesRead;
            }
        }
    }
}

void AudioEngine::decodeStep(Sound& s) {

    AVPacket pkt;
    AVFramePtr frame(av_frame_alloc());

    if (!frame) {
        s.eof = true;
        return;
    }

    if (av_read_frame(s.fmt.get(), &pkt) < 0) {
        s.eof = true;
        return;
    }

    if (pkt.stream_index == s.streamIndex) {
        if (avcodec_send_packet(s.dec.get(), &pkt) >= 0) {
            while (avcodec_receive_frame(s.dec.get(), frame.get()) == 0) {
                // outFrames is the number of FRAMES we'll get after resampling
                int outFrames = swr_get_out_samples(s.swr.get(), frame->nb_samples);

                if (outFrames > 0) {
                    // Calculate total samples needed (frames * channels)
                    size_t totalSamples = outFrames * s.channels;
                    
                    // Check if ring buffer has space
                    if (s.ring->space() < totalSamples) {
                        break; // Ring buffer full, stop decoding
                    }
                    
                    // Ensure decode buffer is large enough (dynamically resize if needed)
                    if (s.decodeBuffer.size() < totalSamples) {
                        s.decodeBuffer.resize(totalSamples);
                    }
                    
                    ui8* outPtrs[] = {
                        reinterpret_cast<ui8*>(s.decodeBuffer.data())
                    };

                    // swr_convert returns number of FRAMES converted
                    int framesConverted = swr_convert(
                        s.swr.get(),
                        outPtrs, outFrames,
                        (const uint8_t**)frame->data,
                        frame->nb_samples);

                    if (framesConverted > 0) {
                        // Write SAMPLES to ring buffer (frames * channels)
                        size_t samplesToWrite = framesConverted * s.channels;
                        s.ring->write(s.decodeBuffer.data(), samplesToWrite);
                    }
                }
            }
        }
    }

    av_packet_unref(&pkt);
}

}
