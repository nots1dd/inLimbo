#include "taglib/Utils.hpp"
#include "taglib/Properties.hpp"

namespace taglib::utils
{

auto parseFractionField(const TagLib::String& text) -> std::pair<int, int>
{
  auto parts = text.split('/');
  int  a     = 0;
  int  b     = 0;
  if (!parts.isEmpty())
    a = parts[0].toInt();
  if (parts.size() > 1)
    b = parts[1].toInt();
  return {a, b};
}

void extractTrackAndTotal(TagLib::FileRef& file, Metadata& metadata)
{
  TagLib::PropertyMap props = file.file()->properties();

  metadata.track      = 0;
  metadata.trackTotal = 0;

  // -------------------------
  // TRACKNUMBER: "X" or "X/Y"
  // -------------------------
  if (hasProp(props, PropKey::TrackNumber))
  {
    auto [t, total]     = parseFractionField(getProp(props, PropKey::TrackNumber));
    metadata.track      = t;
    metadata.trackTotal = total;
  }

  // -------------------------
  // Fallback: TRACKTOTAL / TOTALTRACKS
  // -------------------------
  if (metadata.trackTotal == 0)
  {
    if (hasProp(props, PropKey::TrackTotal))
      metadata.trackTotal = getPropInt(props, PropKey::TrackTotal);
    else if (hasProp(props, PropKey::TotalTracks))
      metadata.trackTotal = getPropInt(props, PropKey::TotalTracks);
  }
}

void extractDiscAndTotal(TagLib::FileRef& file, Metadata& metadata)
{
  TagLib::PropertyMap props = file.file()->properties();

  metadata.discNumber = 0;
  metadata.discTotal  = 0;

  // -------------------------
  // DISCNUMBER: "X" or "X/Y"
  // -------------------------
  if (hasProp(props, PropKey::DiscNumber))
  {
    auto [d, total]     = parseFractionField(getProp(props, PropKey::DiscNumber));
    metadata.discNumber = d;
    metadata.discTotal  = total;
  }

  // -------------------------
  // Fallback: DISCTOTAL / TOTALDISCS
  // -------------------------
  if (metadata.discTotal == 0)
  {
    if (hasProp(props, PropKey::DiscTotal))
      metadata.discTotal = getPropInt(props, PropKey::DiscTotal);
    else if (hasProp(props, PropKey::TotalDiscs))
      metadata.discTotal = getPropInt(props, PropKey::TotalDiscs);
  }
}

void extractAudioProperties(TagLib::FileRef& file, Metadata& metadata)
{
  if (!file.isNull())
  {
    auto* audioProps = file.audioProperties();
    if (audioProps)
    {
      metadata.duration = audioProps->lengthInSeconds();
      metadata.bitrate  = audioProps->bitrate();
    }
  }
}

} // namespace taglib::utils
