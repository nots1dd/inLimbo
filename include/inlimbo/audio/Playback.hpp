#pragma once

#include "Config.hpp"
#include "StackTrace.hpp"
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cassert>
#include "Logger.hpp"

namespace audio {

class AudioEngine {
public:
    struct DeviceInfo {
        std::string name;
        ma_device_id id;
    };

private:
    ma_context context_{};
    ma_resource_manager resourceManager_{};

    // Store objects as unique_ptr to ensure stable addresses (miniaudio structs are not movable!)
    std::vector<std::unique_ptr<ma_engine>> engines_;
    std::vector<std::unique_ptr<ma_device>> devices_;
    std::vector<std::unique_ptr<ma_sound>>  sounds_;

public:
    AudioEngine() {
        LOG_INFO("Initializing miniaudio context...");
        ma_context_config config = ma_context_config_init();
        if (ma_context_init(nullptr, 0, &config, &context_) != MA_SUCCESS) {
            LOG_CRITICAL("Failed to initialize miniaudio context");
            throw std::runtime_error("Failed to initialize miniaudio context");
        }

        ma_resource_manager_config rmConfig = ma_resource_manager_config_init();
        rmConfig.decodedFormat     = ma_format_f32;
        rmConfig.decodedSampleRate = 48000;

        LOG_INFO("Initializing resource manager (48000 Hz, float32)...");
        if (ma_resource_manager_init(&rmConfig, &resourceManager_) != MA_SUCCESS) {
            LOG_CRITICAL("Failed to initialize resource manager");
            throw std::runtime_error("Failed to initialize resource manager");
        }

        LOG_DEBUG("AudioEngine successfully initialized.");
    }

    ~AudioEngine() noexcept {
        RECORD_FUNC_TO_BACKTRACE("AudioEngine::~AudioEngine");
        try {
            cleanup();
            ma_context_uninit(&context_);
            ma_resource_manager_uninit(&resourceManager_);
        } catch (const std::exception& e) {
            LOG_ERROR("Exception during AudioEngine destruction: {}", e.what());
        }
    }

    [[nodiscard]] auto enumeratePlaybackDevices() -> std::vector<DeviceInfo> {
        RECORD_FUNC_TO_BACKTRACE("AudioEngine::enumeratePlaybackDevices");
        LOG_TRACE("Enumerating playback devices...");
        ma_device_info* pPlaybackDeviceInfos = nullptr;
        ma_uint32 playbackDeviceCount = 0;

        if (ma_context_get_devices(const_cast<ma_context*>(&context_),
                                   &pPlaybackDeviceInfos, &playbackDeviceCount,
                                   nullptr, nullptr) != MA_SUCCESS) {
            LOG_ERROR("Failed to get playback devices.");
            throw std::runtime_error("Failed to get playback devices");
        }

        std::vector<DeviceInfo> list;
        for (ma_uint32 i = 0; i < playbackDeviceCount; ++i) {
            list.push_back({pPlaybackDeviceInfos[i].name, pPlaybackDeviceInfos[i].id});
            LOG_DEBUG("Found device: {}", pPlaybackDeviceInfos[i].name);
        }

        LOG_INFO("Found {} playback devices.", list.size());
        return list;
    }

    void initEngineForDevice(const ma_device_id* deviceId) {
        RECORD_FUNC_TO_BACKTRACE("AudioEngine::initEngineForDevice");
        LOG_INFO("Initializing playback device...");

        assert(deviceId != nullptr && "initEngineForDevice called with nullptr deviceId");

        auto device = std::make_unique<ma_device>();
        auto engine = std::make_unique<ma_engine>();

        ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
        deviceConfig.playback.pDeviceID = deviceId;
        deviceConfig.playback.format    = resourceManager_.config.decodedFormat;
        deviceConfig.playback.channels  = 0;
        deviceConfig.sampleRate         = resourceManager_.config.decodedSampleRate;
        deviceConfig.dataCallback       = dataCallback;
        deviceConfig.pUserData          = nullptr;

        ma_result result = ma_device_init(&context_, &deviceConfig, device.get());
        assert(result == MA_SUCCESS && "ma_device_init failed");
        if (result != MA_SUCCESS) {
            LOG_CRITICAL("Failed to initialize playback device.");
            throw std::runtime_error("Failed to initialize playback device");
        }

        ma_engine_config engineConfig = ma_engine_config_init();
        engineConfig.pDevice          = device.get();
        engineConfig.pResourceManager = &resourceManager_;
        engineConfig.noAutoStart      = MA_TRUE;

        result = ma_engine_init(&engineConfig, engine.get());
        assert(result == MA_SUCCESS && "ma_engine_init failed");
        if (result != MA_SUCCESS) {
            LOG_ERROR("Failed to initialize engine for device.");
            ma_device_uninit(device.get());
            throw std::runtime_error("Failed to initialize engine for device");
        }

        device->pUserData = engine.get();
        assert(device->pUserData == engine.get() && "pUserData pointer assignment failed!");

        result = ma_engine_start(engine.get());
        assert(result == MA_SUCCESS && "ma_engine_start failed");
        if (result != MA_SUCCESS) {
            LOG_ERROR("Failed to start audio engine.");
            ma_engine_uninit(engine.get());
            ma_device_uninit(device.get());
            throw std::runtime_error("Failed to start engine");
        }

        LOG_INFO("Playback engine started successfully.");

        devices_.push_back(std::move(device));
        engines_.push_back(std::move(engine));

        assert(devices_.size() == engines_.size() && "Device/Engine arrays should match 1:1");
        assert(devices_.back()->pUserData != nullptr && "Device pUserData should not be null after init");
    }

    void loadSound(const std::string& filepath, size_t engineIndex = 0) {
        RECORD_FUNC_TO_BACKTRACE("AudioEngine::loadSound");
        assert(!filepath.empty() && "Empty filepath passed to loadSound()");
        assert(engineIndex < engines_.size() && "Engine index out of range before loadSound");

        LOG_INFO("Loading sound: {}", filepath);

        auto sound = std::make_unique<ma_sound>();
        ma_result result = ma_sound_init_from_file(
            engines_[engineIndex].get(), filepath.c_str(),
            MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE |
            MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC |
            MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_STREAM,
            nullptr, nullptr, sound.get());

        assert(result == MA_SUCCESS && "ma_sound_init_from_file failed");
        if (result != MA_SUCCESS) {
            LOG_ERROR("Failed to load sound: {}", filepath);
            throw std::runtime_error("Failed to load sound: " + filepath);
        }

        LOG_DEBUG("Sound loaded successfully: {}", filepath);
        sounds_.push_back(std::move(sound));
    }

    // --- Playback Controls ---

    void play(size_t soundIndex = 0) {
        RECORD_FUNC_TO_BACKTRACE("AudioEngine::play");
        assert(soundIndex < sounds_.size() && "Invalid sound index in play()");
        LOG_INFO("Playing sound index {}", soundIndex);
        ma_sound_start(sounds_[soundIndex].get());
    }

    void pause(size_t soundIndex = 0) {
        assert(soundIndex < sounds_.size() && "Invalid sound index in pause()");
        LOG_INFO("Pausing sound index {}", soundIndex);
        ma_sound_stop(sounds_[soundIndex].get());
    }

    void restart(size_t soundIndex = 0) {
        assert(soundIndex < sounds_.size() && "Invalid sound index in restart()");
        LOG_INFO("Restarting sound index {}", soundIndex);
        ma_sound_seek_to_pcm_frame(sounds_[soundIndex].get(), 0);
        ma_sound_start(sounds_[soundIndex].get());
    }

    void seekForward(size_t soundIndex, ma_uint64 frames) {
        assert(soundIndex < sounds_.size() && "Invalid sound index in seekForward()");
        ma_uint64 cursor{};
        ma_sound_get_cursor_in_pcm_frames(sounds_[soundIndex].get(), &cursor);
        ma_sound_seek_to_pcm_frame(sounds_[soundIndex].get(), cursor + frames);
        LOG_DEBUG("Seeked forward {} frames on sound {}", frames, soundIndex);
    }

    void seekBackward(size_t soundIndex, ma_uint64 frames) {
        RECORD_FUNC_TO_BACKTRACE("AudioEngine::seekBackward");
        assert(soundIndex < sounds_.size() && "Invalid sound index in seekBackward()");
        ma_uint64 cursor{};
        ma_sound_get_cursor_in_pcm_frames(sounds_[soundIndex].get(), &cursor);
        ma_sound_seek_to_pcm_frame(sounds_[soundIndex].get(), cursor > frames ? cursor - frames : 0);
        LOG_DEBUG("Seeked backward {} frames on sound {}", frames, soundIndex);
    }

    void setVolume(float volume, size_t soundIndex = 0) {
        RECORD_FUNC_TO_BACKTRACE("AudioEngine::setVolume");
        assert(soundIndex < sounds_.size() && "Invalid sound index in setVolume()");
        assert(volume >= 0.0f && volume <= 1.5f && "Volume out of safe range (0.0 - 1.5)");
        LOG_INFO("Setting volume of sound {} to {:.2f}", soundIndex, volume);
        ma_sound_set_volume(sounds_[soundIndex].get(), volume);
    }

    void printDeviceInfo(size_t deviceIndex = 0) const {
        RECORD_FUNC_TO_BACKTRACE("AudioEngine::printDeviceInfo");
        assert(deviceIndex < devices_.size() && "Invalid device index in printDeviceInfo()");
        const auto& dev = *devices_[deviceIndex];
        LOG_INFO("Device Info -> Name: {}, Channels: {}, SampleRate: {}",
                 dev.playback.name, dev.playback.channels, dev.sampleRate);
    }

    void cleanup() {
        RECORD_FUNC_TO_BACKTRACE("AudioEngine::cleanup");
        LOG_DEBUG("Cleaning up audio resources...");

        LOG_DEBUG("Cleanup: {} sounds, {} engines, {} devices",
                  sounds_.size(), engines_.size(), devices_.size());

        if (sounds_.empty() || (engines_.empty() && devices_.empty())) {
            LOG_DEBUG("No audio resources to clean up.");
            return;
        }

        for (auto& sound : sounds_) {
            ma_sound_uninit(sound.get());
        }
        sounds_.clear();

        for (auto& engine : engines_) {
            assert(engine->pDevice != nullptr && "Engine has null device pointer during cleanup");
            ma_engine_uninit(engine.get());
        }
        engines_.clear();

        for (auto& device : devices_) {
            ma_device_state state = ma_device_get_state(device.get());
            LOG_DEBUG("Device cleanup: state={}", static_cast<int>(state));
            if (state == ma_device_state_started) {
                LOG_WARN("Device still started at cleanup! Forcing stop...");
                ma_device_stop(device.get());
            }
            ma_device_uninit(device.get());
        }
        devices_.clear();

        LOG_INFO("Audio cleanup completed successfully.");
    }

    void startInteractiveLoop(std::function<void(AudioEngine&)> loopLogic) {
        stopInteractiveLoop(); // stop old loop if any

        shouldRun_.store(true);
        loopThread_ = std::thread([this, loopLogic = std::move(loopLogic)] {
            try {
                loopLogic(*this);
            } catch (const std::exception& e) {
                std::cerr << "[AudioEngine] Loop error: " << e.what() << '\n';
            }
        });
    }

    void stopInteractiveLoop() {
        RECORD_FUNC_TO_BACKTRACE("AudioEngine::stopInteractiveLoop");
        shouldRun_.store(false);
        if (loopThread_.joinable())
            loopThread_.join();
    }

    [[nodiscard]] auto shouldRun() const noexcept -> bool {
        return shouldRun_.load();
    }

private:
    std::atomic<bool> shouldRun_{false};
    std::thread loopThread_;

    static void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
        (void)pInput;
        assert(pDevice != nullptr && "dataCallback got null device pointer");
        auto* engine = recast<ma_engine*>(pDevice->pUserData);
        assert(engine != nullptr && "dataCallback: pUserData is null!");
        ma_engine_read_pcm_frames(engine, pOutput, frameCount, nullptr);
    }
};

} // namespace audio
