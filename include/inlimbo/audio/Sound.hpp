#pragma once

#include <atomic>
#include <memory>
#include "utils/RingBuffer.hpp"
#include "InLimbo-Types.hpp"

extern "C" 
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
}

#define SOUND_BUFFER_SIZE_FRAMES  (1 << 16)

namespace audio
{

struct Sound {
    AVFormatContext* fmt = nullptr;
    AVCodecContext*  dec = nullptr;
    AVStream* stream = nullptr;
    SwrContext* swr = nullptr;

    int streamIndex = -1;
    int sampleRate = 48000;
    int channels = 2;

    i64 durationFrames = 0;
    std::atomic<i64> cursorFrames{0};

    i64 startSkip = 0;
    i64 endSkip   = 0;

    std::atomic<bool> eof{false};

    std::atomic<bool> seekPending{false};
    std::atomic<i64>  seekTargetFrame{0};

    RingBuffer ring;

    Sound() : ring(SOUND_BUFFER_SIZE_FRAMES) {}
};

using Sounds_ptr = std::vector<std::unique_ptr<Sound>>;
using Sounds = std::vector<Sound>;

}
