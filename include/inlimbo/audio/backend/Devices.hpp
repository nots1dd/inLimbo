#pragma once

#include "audio/Sound.hpp"
#include "audio/backend/alsa/BackendInfo.hpp"
#include "utils/string/SmallString.hpp"
#include <variant>

namespace audio
{

using DeviceName = utils::string::SmallString;
using CodecName  = utils::string::SmallString;

struct Device
{
  DeviceName                 name;
  utils::string::SmallString description;
  int                        cardIndex   = -1;
  int                        deviceIndex = -1;
  bool                       isDefault   = false;
};

struct BackendCommonInfo
{
  Device dev;

  // ---------------------------------------------------------
  // Audio format (NEGOTIATED, not requested)
  // ---------------------------------------------------------
  uint                       sampleRate = DEFAULT_SOUND_SAMPLE_RATE;
  uint                       channels   = DEFAULT_SOUND_CHANNELS;
  utils::string::SmallString pcmFormatName;

  CodecName codecName;     // "flac", "mp3", "aac", ...
  CodecName codecLongName; // "FLAC (Free Lossless Audio Codec)", etc.

  double latencyMs = 0.0;

  bool isActive   = false;
  bool isPlaying  = false;
  bool isPaused   = false;
  bool isDraining = false;

  ui64 xruns = 0; // underrun count (https://unix.stackexchange.com/questions/199498/what-are-xruns)
  ui64 writes = 0; // audio backend write calls (ex: snd_pcm_writei for ALSA)
};

using Devices = std::vector<Device>;

using BackendSpecificInfo = std::variant<std::monostate, backend::AlsaBackendInfo>;

struct BackendInfo
{
  BackendCommonInfo   common;
  BackendSpecificInfo specific;
};

} // namespace audio
