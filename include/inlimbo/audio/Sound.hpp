#pragma once
#include "InLimbo-Types.hpp"
#include "utils/RingBuffer.hpp"
#include <atomic>
#include <memory>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
}

#define DEFAULT_SOUND_SAMPLE_RATE 48000
#define DEFAULT_SOUND_CHANNELS    2
#define SOUND_BUFFER_SIZE_FRAMES  (1 << 16)

// Configuration for buffer sizing
#define RING_BUFFER_SECONDS      5.0  // Ring buffer holds N seconds of audio
#define DECODE_BUFFER_SECONDS    0.5  // Decode buffer holds N seconds of audio
#define MIN_DECODE_BUFFER_FRAMES 4096 // Minimum decode buffer size in frames

namespace audio
{

// RAII Wrappers for FFmpeg resources
struct AVFormatContextDeleter
{
  void operator()(AVFormatContext* ctx) const
  {
    if (ctx)
      avformat_close_input(&ctx);
  }
};

struct AVCodecContextDeleter
{
  void operator()(AVCodecContext* ctx) const
  {
    if (ctx)
      avcodec_free_context(&ctx);
  }
};

struct SwrContextDeleter
{
  void operator()(SwrContext* ctx) const
  {
    if (ctx)
      swr_free(&ctx);
  }
};

struct AVFrameDeleter
{
  void operator()(AVFrame* frame) const
  {
    if (frame)
      av_frame_free(&frame);
  }
};

using AVFormatContextPtr = std::unique_ptr<AVFormatContext, AVFormatContextDeleter>;
using AVCodecContextPtr  = std::unique_ptr<AVCodecContext, AVCodecContextDeleter>;
using SwrContextPtr      = std::unique_ptr<SwrContext, SwrContextDeleter>;
using AVFramePtr         = std::unique_ptr<AVFrame, AVFrameDeleter>;

struct AudioFormat
{
  uint            sampleRate;
  uint            channels;
  AVSampleFormat  sampleFmt;
  AVChannelLayout channelLayout;

  std::string sampleFmtName;
};

struct Sound
{
  // Sound struct is NOT trivially copyable
  Sound(const Sound&)                        = delete;
  auto operator=(const Sound&) -> Sound&     = delete;
  Sound(Sound&&) noexcept                    = delete;
  auto operator=(Sound&&) noexcept -> Sound& = delete;

  AudioFormat source; // exact file properties
  AudioFormat target; // engine output format (matches backend)

  AVFormatContextPtr fmt;
  AVCodecContextPtr  dec;
  SwrContextPtr      swr;
  AVStream*          stream = nullptr;

  int streamIndex = -1;
  int sampleRate  = 0;
  int channels    = 2;

  i64 startSkip      = 0;
  i64 endSkip        = 0;
  i64 durationFrames = 0;

  std::atomic<i64>  cursorFrames{0};
  std::atomic<i64>  seekTargetFrame{0};
  std::atomic<bool> seekPending{false};
  std::atomic<bool> eof{false};

  // Dynamic ring buffer - size calculated in constructor
  std::unique_ptr<utils::RingBuffer<float>> ring;

  // Reusable decode buffer - size calculated based on audio params
  std::vector<float> decodeBuffer;

  Sound() = default;

  // Initialize buffers after audio parameters are known
  void initializeBuffers()
  {
    if (sampleRate <= 0 || channels <= 0)
    {
      throw std::runtime_error("Invalid audio parameters for buffer initialization");
    }

    // Ring buffer: RING_BUFFER_SECONDS of audio at current sample rate
    // Size in samples = seconds * sampleRate * channels
    auto ringBufferSamples = static_cast<size_t>(RING_BUFFER_SECONDS * sampleRate * channels);
    ring                   = std::make_unique<utils::RingBuffer<float>>(ringBufferSamples);

    // Decode buffer: DECODE_BUFFER_SECONDS of audio or MIN_DECODE_BUFFER_FRAMES, whichever is
    // larger This ensures we can handle large codec frames
    size_t decodeBufferFrames  = std::max(static_cast<size_t>(DECODE_BUFFER_SECONDS * sampleRate),
                                          static_cast<size_t>(MIN_DECODE_BUFFER_FRAMES));
    size_t decodeBufferSamples = decodeBufferFrames * channels;
    decodeBuffer.reserve(decodeBufferSamples);
    decodeBuffer.resize(decodeBufferSamples);
  }

  // Get ring buffer capacity in frames (for readability)
  [[nodiscard]] auto getRingCapacityFrames() const -> size_t
  {
    return ring ? ring->capacity() / channels : 0;
  }

  // Get ring buffer available data in frames
  [[nodiscard]] auto getRingAvailableFrames() const -> size_t
  {
    return ring ? ring->available() / channels : 0;
  }

  // Get ring buffer free space in frames
  [[nodiscard]] auto getRingSpaceFrames() const -> size_t
  {
    return ring ? ring->space() / channels : 0;
  }
};

using Sounds_ptr = std::vector<std::unique_ptr<Sound>>;

} // namespace audio
