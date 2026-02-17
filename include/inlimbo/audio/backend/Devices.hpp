#pragma once

#include "utils/string/SmallString.hpp"
#include <vector>

namespace audio
{

using DeviceName = utils::string::SmallString;
using CodecName  = utils::string::SmallString;

using DeviceDescription = utils::string::SmallString;

struct Device
{
  DeviceName        name;
  DeviceDescription description;
  int               cardIndex   = -1;
  int               deviceIndex = -1;
  bool              isDefault   = false;
};

using Devices = std::vector<Device>;

} // namespace audio
