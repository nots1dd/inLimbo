#pragma once

#include <cstddef>

namespace audio::constants
{

// Frames written per snd_pcm_writei() call
inline constexpr std::size_t FramesPerBuffer = 512;

inline constexpr std::size_t MaxChannels = 8;

inline constexpr float FloatMin = -1.0f;
inline constexpr float FloatMax = +1.0f;

// --------------------------------------
// Float -> PCM integer scaling
// --------------------------------------

// NOTE: int16 range = [-32768..32767]
// We scale by max positive.
inline constexpr float S16MaxFloat = 32767.0f;

// NOTE: int32 range = [-2147483648..2147483647]
inline constexpr float S32MaxFloat = 2147483647.0f;

inline constexpr int SwrRoundMode = 1; // AV_ROUND_UP decl (without using libav)

} // namespace audio::constants
