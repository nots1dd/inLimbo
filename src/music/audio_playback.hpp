#ifndef AUDIO_PLAYBACK_HPP
#define AUDIO_PLAYBACK_HPP

#include <iostream>
#include <string>
#include <stdexcept>
#include <thread>
#include <chrono>
#include "miniaudio.h"

class MiniAudioPlayer {
private:
    ma_engine engine;
    ma_sound sound;
    bool isPlaying;

public:
    MiniAudioPlayer() : isPlaying(false) {
        if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
            throw std::runtime_error("Failed to initialize MiniAudio engine.");
        }
    }

    ~MiniAudioPlayer() {
        ma_sound_uninit(&sound);
        ma_engine_uninit(&engine);
    }

    void loadFile(const std::string &filePath) {
        if (isPlaying) {
            stop();
        }

        if (ma_sound_init_from_file(&engine, filePath.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, &sound) != MA_SUCCESS) {
            throw std::runtime_error("Failed to load audio file: " + filePath);
        }
    }

    void play() {
        if (!isPlaying) {
            if (ma_sound_start(&sound) != MA_SUCCESS) {
                throw std::runtime_error("Failed to play the sound.");
            }
            isPlaying = true;
        }
    }

    void pause() {
        if (isPlaying) {
            if (ma_sound_stop(&sound) != MA_SUCCESS) {
                throw std::runtime_error("Failed to pause the sound.");
            }
            isPlaying = false;
        }
    }

    void resume() {
        play();
    }

    void stop() {
        if (isPlaying) {
            if (ma_sound_stop(&sound) != MA_SUCCESS) {
                throw std::runtime_error("Failed to stop the sound.");
            }
            isPlaying = false;
        }
        ma_sound_uninit(&sound);
    }

    void setVolume(float volume) {
        if (volume < 0.0f || volume > 1.0f) {
            throw std::invalid_argument("Volume must be between 0.0 and 1.0.");
        }
        ma_sound_set_volume(&sound, volume);
    }

    float getVolume() const {
        return ma_sound_get_volume(&sound);
    }

    bool isCurrentlyPlaying() const {
        return isPlaying;
    }
};


#endif
