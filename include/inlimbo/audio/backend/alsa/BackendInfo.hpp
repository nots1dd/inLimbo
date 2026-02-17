#pragma once

#include "utils/string/SmallString.hpp"

extern "C"
{
#include <alsa/asoundlib.h>
}

namespace audio::backend
{

struct AlsaBackendInfo
{
  snd_pcm_format_t           pcmFormat = SND_PCM_FORMAT_UNKNOWN;
  utils::string::SmallString pcmFormatName;

  snd_pcm_uframes_t periodSize = 0;
  snd_pcm_uframes_t bufferSize = 0;

  bool isDraining = false;
};

} // namespace audio::backend
