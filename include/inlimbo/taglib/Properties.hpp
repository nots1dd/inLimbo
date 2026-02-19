#pragma once

#include <array>
#include <taglib/tpropertymap.h>

namespace taglib
{

enum class PropKey : uint8_t
{
  TrackNumber = 0,
  TrackTotal,
  TotalTracks,
  DiscNumber,
  DiscTotal,
  TotalDiscs,
  AlbumArtist,
  Lyrics,
  COUNT
};

constexpr std::array<std::string_view, static_cast<size_t>(PropKey::COUNT)> PROP_KEY_TEXT = {
  "TRACKNUMBER", "TRACKTOTAL", "TOTALTRACKS", "DISCNUMBER",
  "DISCTOTAL",   "TOTALDISCS", "ALBUMARTIST", "LYRICS"};

static inline auto propText(PropKey k) -> std::string_view
{
  return PROP_KEY_TEXT[static_cast<size_t>(k)];
}

static inline auto propTagString(PropKey k) -> TagLib::String
{
  auto t = propText(k);
  return {t.data(), TagLib::String::UTF8};
}

static inline auto hasProp(const TagLib::PropertyMap& props, PropKey k) -> bool
{
  return props.contains(propTagString(k));
}

static inline auto getProp(const TagLib::PropertyMap& props, PropKey k) -> TagLib::String
{
  return props[propTagString(k)].toString();
}

static inline auto getPropInt(const TagLib::PropertyMap& props, PropKey k) -> int
{
  return getProp(props, k).toInt();
}

} // namespace taglib
