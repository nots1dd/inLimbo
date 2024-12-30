#ifndef AUDIO_PLAYBACK_HPP
#define AUDIO_PLAYBACK_HPP
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

class MiniAudioPlayer
{
private:
  ma_engine   engine;
  ma_sound    sound;
  bool        isPlaying;
  bool        wasPaused;
  uint64_t    pausePosition; // To store the position where the sound was paused
  uint64_t    currentPosition;
  std::thread playbackThread;

public:
  MiniAudioPlayer() : isPlaying(false)
  {
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS)
    {
      throw std::runtime_error("Failed to initialize MiniAudio engine.");
    }
  }

  ~MiniAudioPlayer()
  {
    stop();
    ma_sound_uninit(&sound);
    ma_engine_uninit(&engine);
  }

  int loadFile(const std::string& filePath)
  {
    if (isPlaying)
    {
      stop();
    }

    ma_sound_uninit(&sound);

    if (ma_sound_init_from_file(&engine, filePath.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL,
                                &sound) != MA_SUCCESS)
    {
      throw std::runtime_error("Failed to load audio file: " + filePath);
      return -1;
    }
    std::cout << "Loaded " << filePath << " with MA_SUCCESS" << std::endl;
    return 0;
  }

  void play()
  {
    if (!isPlaying)
    {
      if (ma_sound_start(&sound) != MA_SUCCESS)
      {
        throw std::runtime_error("Failed to play the sound.");
      }
      isPlaying = true;

      // Stop any existing playback thread
      if (playbackThread.joinable())
      {
        playbackThread.join();
      }

      // Start a new playback thread
      playbackThread = std::thread(
        [this]()
        {
          while (isPlaying)
          {
            if (!ma_sound_is_playing(&sound))
            {
              isPlaying = false;
              break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
          }
        });
    }
  }

  void pause()
  {
    if (isPlaying)
    {
      // Store the current position before stopping
      pausePosition = ma_sound_get_time_in_pcm_frames(&sound);

      if (ma_sound_stop(&sound) != MA_SUCCESS)
      {
        throw std::runtime_error("Failed to pause the sound.");
      }
      isPlaying = false;
      wasPaused = true;

      if (playbackThread.joinable())
      {
        playbackThread.join();
      }
    }
  }

  void resume()
  {
    if (wasPaused)
    {
      // Seek to the stored position and start playing again
      ma_sound_seek_to_pcm_frame(&sound, pausePosition);

      play(); // Call play() to resume playback from the correct position
      wasPaused = false;
    }
  }

  void stop()
  {
    if (isPlaying)
    {
      if (ma_sound_stop(&sound) != MA_SUCCESS)
      {
        throw std::runtime_error("Failed to stop the sound.");
      }
      isPlaying = false;
      if (playbackThread.joinable())
      {
        playbackThread.join();
      }
      ma_sound_seek_to_pcm_frame(&sound, 0);
    }
  }

  void setVolume(float volume)
  {
    if (volume < 0.0f || volume > 1.0f)
    {
      throw std::invalid_argument("Volume must be between 0.0 and 1.0.");
    }
    ma_sound_set_volume(&sound, volume);
  }

  float getVolume() const { return ma_sound_get_volume(&sound); }

  bool isCurrentlyPlaying() const { return isPlaying; }

  float getDuration()
  {
    if (ma_sound_get_time_in_milliseconds(&sound) != MA_SUCCESS)
    {
      throw std::runtime_error("Failed to get sound duration.");
    }
    float     duration = 0.0f;
    ma_result result   = ma_sound_get_length_in_seconds(&sound, &duration);
    if (result != MA_SUCCESS)
    {
      throw std::runtime_error("Failed to get sound duration. Result: " + std::to_string(result));
    }
    return duration;
  }
  // BROKEN (NOT STRAIGHTFORWARD ITS ALL IN PCM TERMS)
  double seekTime(int diff)
  {
      // Get the current position in PCM frames
      currentPosition = ma_sound_get_time_in_pcm_frames(&sound);

      // Get the total length of the song in PCM frames
      ma_uint64 songLengthInFrames;
      ma_result result = ma_sound_get_length_in_pcm_frames(&sound, &songLengthInFrames);

      if (result != MA_SUCCESS)
      {
          throw std::runtime_error("Failed to get the length of the sound in PCM frames.");
      }

      uint32_t sampleRate = ma_engine_get_sample_rate(&engine);

      uint64_t diffInFrames = static_cast<uint64_t>(diff * sampleRate);

      currentPosition += diffInFrames;

      if (currentPosition < 0)
      {
          currentPosition = 0;
      }
      else if (currentPosition > songLengthInFrames)
      {
          currentPosition = songLengthInFrames;
      }

      result = ma_sound_seek_to_pcm_frame(&sound, currentPosition);

      if (result != MA_SUCCESS)
      {
          throw std::runtime_error("Failed to seek to the PCM frame.");
      }

      play();

      return static_cast<double>(currentPosition);
  }

};
#endif
