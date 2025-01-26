#ifndef AUDIO_PLAYBACK_HPP
#define AUDIO_PLAYBACK_HPP

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <chrono>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <future>
#include <filesystem>

/**
 * @struct AudioDevice
 * @brief A structure representing an audio playback device.
 */
struct AudioDevice
{
  std::string  name; /**< The name of the audio device. */
  ma_device_id id;   /**< The MiniAudio device ID. */
};

/**
 * @class MiniAudioPlayer
 * @brief A class for playing audio using the MiniAudio library.
 *
 * This class wraps the functionality of the MiniAudio library to provide methods for loading,
 * playing, pausing, resuming, stopping, adjusting volume, and seeking within an audio file. It
 * manages playback in a separate thread to handle real-time updates on whether the sound is
 * currently playing.
 */
class MiniAudioPlayer
{
private:
  ma_engine        engine; /**< The MiniAudio engine object used to manage sound playback. */
  ma_sound         sound;  /**< The sound object representing the audio file being played. */
  ma_device        device;
  ma_context       context;
  ma_engine_config engine_config;
  ma_device_id     deviceID; /**< Selected audio device ID for playback. */
  ma_device_config deviceConfig;
  ma_sound_config  soundConfig;
  bool             isPlaying; /**< Flag to track if the sound is currently playing. */
  bool             deviceSet; /**< Flag to check if an audio device is set or not. */
  bool             wasPaused; /**< Flag to track if the sound was paused. */
  uint64_t    pausePosition;  /**< Stores the position in PCM frames where the sound was paused. */
  std::thread playbackThread, audioDeviceThread; /**< A thread for monitoring the playback state in the background. */
  std::mutex  mtx, devicesMutex;            /**< Mutex to protect shared state between methods and threads. */
  std::vector<AudioDevice> devices;

public:
  /**
   * @brief Constructs a MiniAudioPlayer object and initializes the MiniAudio engine.
   *
   * Initializes the engine and prepares the player for loading and playing audio.
   *
   * @throws std::runtime_error If the MiniAudio engine cannot be initialized.
   */
  MiniAudioPlayer() : isPlaying(false), wasPaused(false), pausePosition(0), deviceSet(false)
  {
    engine_config = ma_engine_config_init();
    if (ma_engine_init(&engine_config, &engine) != MA_SUCCESS)
    {
      throw std::runtime_error("Failed to initialize MiniAudio engine.");
    }

    deviceConfig                    = ma_device_config_init(ma_device_type_playback);
    if (ma_context_init(nullptr, 0, nullptr, &context) != MA_SUCCESS)
    {
      throw std::runtime_error("Failed to initialize audio context.");
    }

    if (ma_device_init(nullptr, &deviceConfig, &device) != MA_SUCCESS) 
    {
        throw std::runtime_error("Failed to initialize playback device.");
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        ma_device_uninit(&device);
        throw std::runtime_error("Failed to start playback device.");
    }
  }

  /**
   * @brief Destroys the MiniAudioPlayer object, stopping any playback and cleaning up resources.
   *
   * This method ensures that the sound is stopped, the engine is uninitialized, and resources are
   * cleaned up.
   */
  ~MiniAudioPlayer()
  {
    stop();
    ma_device_uninit(&device);
    ma_context_uninit(&context);
    ma_engine_uninit(&engine);
  }

  auto enumerateDevices() -> std::vector<AudioDevice>
  {
      std::vector<AudioDevice> localDevices;

      audioDeviceThread = std::thread([this, &localDevices]() {
          try {
              ma_device_info* playbackDevices;
              ma_uint32 playbackDeviceCount;

              // Enumerate devices
              ma_result result = ma_context_get_devices(&context, &playbackDevices, &playbackDeviceCount, nullptr, nullptr);
              if (result != MA_SUCCESS)
              {
                  throw std::runtime_error("Failed to enumerate playback devices.");
              }

              // Log device info for debugging
              std::cout << "Found " << playbackDeviceCount << " playback devices:\n";
              for (ma_uint32 i = 0; i < playbackDeviceCount; ++i) {
                  std::cout << "Device " << i << ": " << playbackDevices[i].name << std::endl;
              }

              // Safely copy devices
              {
                  std::lock_guard<std::mutex> lock(devicesMutex);
                  devices.clear();
                  for (ma_uint32 i = 0; i < playbackDeviceCount; ++i) {
                      devices.push_back({playbackDevices[i].name, playbackDevices[i].id});
                  }
                  localDevices = devices;
              }

          } catch (const std::exception& e) {
              std::cerr << "Error in enumerateDevices thread: " << e.what() << "\n";
          }
      });

      if (audioDeviceThread.joinable()) audioDeviceThread.join(); // Wait for the thread to finish
      return localDevices;
  }

  /**
   * @brief Sets the device ID for playback.
   *
   * @param id The device ID to set for playback.
   */
  void setDevice(const ma_device_id& id)
  {
    std::unique_lock<std::mutex> lock(mtx);
    deviceID  = id;
    deviceConfig.playback.pDeviceID = &deviceID;
    deviceSet = true;
  }

  /**
   * @brief Loads an audio file for playback.
   *
   * This method loads the audio file into memory for playback. If a file is already loaded, it is
   * unloaded before loading the new file. If the player is currently playing, it stops the playback
   * first.
   *
   * @param filePath The path to the audio file to load.
   * @return 0 on success, -1 on failure.
   * @throws std::runtime_error If the audio file cannot be loaded.
   */
  auto loadFileAsync(const std::string& filePath, bool reloadNextFile) -> std::future<int>
  {
      return std::async(std::launch::async, [this, filePath, reloadNextFile]()
      {
          std::unique_lock<std::mutex> lock(mtx);

          if (isPlaying)
          {
              stop();
          }

          // File check is done before loading the file so this seems like an unnecessary sanity check that can be removed
          /*if (!std::filesystem::exists(filePath))*/
          /*{*/
          /*    throw std::runtime_error("File does not exist: " + filePath);*/
          /*}*/

          if (reloadNextFile)
          {
              ma_sound_uninit(&sound);
          }

          if (deviceSet)
          {
              deviceConfig.playback.pDeviceID = &deviceID;
          }

          if (ma_sound_init_from_file(&engine, filePath.c_str(), MA_SOUND_FLAG_STREAM, nullptr, nullptr, &sound) != MA_SUCCESS)
          {
              throw std::runtime_error("Failed to load audio file: " + filePath);
          }

          return 0; // Success
      });
  }

  /**
   * @brief Starts playback of the loaded audio file.
   *
   * This method begins playback from the beginning of the audio file. If the audio file is already
   * playing, nothing happens. A playback thread is started to monitor the state of playback.
   *
   * @throws std::runtime_error If playback cannot be started.
   */
  void play()
  {
    std::unique_lock<std::mutex> lock(mtx); // Protect shared state
    deviceConfig.playback.pDeviceID = &deviceID;
    if (!isPlaying)
    {
      if (ma_sound_start(&sound) != MA_SUCCESS)
      {
        throw std::runtime_error("Failed to play the sound.");
      }
      isPlaying = true;

      if (playbackThread.joinable()) playbackThread.join();

      // Start a new playback thread
      playbackThread = std::thread(
        [this]()
        {
          while (isPlaying)
          {
            if (!ma_sound_is_playing(&sound))
            {
              std::unique_lock<std::mutex> lock(mtx); // Protect shared state
              isPlaying = false;
              break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
          }
        });
    }
    mtx.unlock();
  }

  /**
   * @brief Pauses the current audio playback.
   *
   * This method pauses playback and stores the current position to allow resumption from the same
   * point.
   *
   * @throws std::runtime_error If pausing the sound fails.
   */
  void pause()
  {
    std::unique_lock<std::mutex> lock(mtx); // Protect shared state
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
    else 
    {
      throw std::runtime_error("No song is playing...");
    }
  }

  /**
   * @brief Resumes playback from the last paused position.
   *
   * This method resumes playback from the position where the sound was paused. If the sound was not
   * paused, nothing happens.
   */
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

  /**
   * @brief Stops the playback and resets the playback position.
   *
   * This method stops the current playback, resets the playback position to the beginning, and
   * ensures the playback thread is joined.
   */
  void stop()
  {
    std::unique_lock<std::mutex> lock(mtx); // Protect shared state
    if (isPlaying)
    {
      if (ma_sound_stop(&sound) != MA_SUCCESS)
      {
        throw std::runtime_error("Failed to stop the sound.");
      }
      isPlaying = false;
      ma_sound_seek_to_pcm_frame(&sound, 0);
    }

    if (playbackThread.joinable())
    {
      playbackThread.join();
    }
  }

  /**
   * @brief Sets the volume of the audio playback.
   *
   * This method adjusts the playback volume. The volume is a float between 0.0 (silent) and 1.0
   * (maximum volume).
   *
   * @param volume The volume to set.
   * @throws std::invalid_argument If the volume is outside the valid range of 0.0 to 1.0.
   */
  void setVolume(float volume)
  {
    if (volume < 0.0f || volume > 1.0f)
    {
      throw std::invalid_argument("Volume must be between 0.0 and 1.0.");
    }
    ma_sound_set_volume(&sound, volume);
  }

  /**
   * @brief Gets the current volume of the audio playback.
   *
   * This method retrieves the current volume level of the sound playback.
   *
   * @return The current volume level.
   */
  float getVolume() const { return ma_sound_get_volume(&sound); }

  /**
   * @brief Checks if the sound is currently playing.
   *
   * This method returns whether the sound is currently playing.
   *
   * @return `true` if the sound is playing, `false` otherwise.
   */
  bool isCurrentlyPlaying()
  {
    std::unique_lock<std::mutex> lock(mtx); // Protect shared state
    return isPlaying;
  }

  /**
   * @brief Gets the total duration of the sound in seconds. (async function)
   *
   * This method retrieves the total duration of the loaded audio file in seconds.
   *
   * @return The duration of the sound in seconds.
   * @throws std::runtime_error If the duration cannot be retrieved.
   */
  auto getDurationAsync() -> std::future<float>
  {
      return std::async(std::launch::async, [this]() {
          std::unique_lock<std::mutex> lock(mtx); // Protect shared state
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
      });
  }

  /**
   * @brief Seeks to a specific time in the audio (in seconds).
   *
   * This method seeks to a specific position in the audio based on the number of seconds provided.
   *
   * @param seconds The time (in seconds) to seek to.
   * @return The actual time (in seconds) that the sound was seeked to.
   */
  double seekTime(int seconds)
  {
    if (isPlaying)
    {
      std::unique_lock<std::mutex> lock(mtx); // Protect shared state

      // Get the sample rate of the sound
      ma_uint32 sampleRate = ma_engine_get_sample_rate(&engine);

      // Get the total length of the sound in PCM frames
      ma_uint64 totalFrames;
      ma_result result = ma_sound_get_length_in_pcm_frames(&sound, &totalFrames);
      if (result != MA_SUCCESS)
      {
        std::cerr << "Failed to get total PCM frames." << std::endl;
      }

      // Get the current position in PCM frames
      ma_uint64 currentFrames = ma_sound_get_time_in_pcm_frames(&sound);

      // Calculate the new position in PCM frames
      ma_int64 newFrames =
        static_cast<ma_int64>(currentFrames) + static_cast<ma_int64>(seconds) * sampleRate;

      // Clamp the new position to valid bounds
      if (newFrames < 0)
        newFrames = seconds = 0;
      if (static_cast<ma_uint64>(newFrames) > totalFrames)
        newFrames = totalFrames;

      result = ma_sound_seek_to_pcm_frame(&sound, static_cast<ma_uint64>(newFrames));
      if (result != MA_SUCCESS)
      {
        std::cerr << "Failed to seek sound." << std::endl;
      }

      return (double)seconds;
    }
    else 
    {  
      throw std::runtime_error("-- Audio is not playing, cannot seek time");
    }
  }
};

#endif
