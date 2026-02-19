#include "taglib/Parser.hpp"
#include "Logger.hpp"
#include "utils/fs/FileUri.hpp"
#include "utils/fs/Paths.hpp"
#include <string_view>
#include <taglib/flacfile.h>
#include <taglib/id3v2.h>
#include <taglib/mpegfile.h>

namespace taglib
{

Parser::Parser(TagLibConfig config) { m_config = std::move(config); }

auto Parser::parseFile(const Path& filePath, Metadata& metadata) -> bool
{
  if (m_config.debugLog)
    LOG_DEBUG("Parsing file: {}", filePath);

  auto ext = filePath.extension();

  auto* source = findSource(ext.c_str());

  if (!source)
  {
    LOG_WARN("No parser strategy for file: {}", filePath);
    return false;
  }

  LOG_DEBUG("Using tag source for extension: {}", ext);

  metadata.filePath = filePath;

  if (!source->parse(filePath, metadata, m_config, m_parseSession))
    return false;

  if (!fillArtUrl(metadata))
  {
    LOG_WARN("No embedded art found for file: {}", filePath);
    metadata.artUrl = "";
  }

  return true;
}

auto Parser::modifyMetadata(const Path& filePath, const Metadata& newData) -> bool
{
  auto* source = findSource(filePath.extension().c_str());

  if (!source)
  {
    LOG_CRITICAL("Unsupported file type for metadata modification: {}", filePath.extension());
    return false;
  }

  try
  {
    bool ok = source->modify(filePath, newData, m_config);
    if (m_config.debugLog && ok)
      LOG_INFO("Metadata updated successfully for: {}", filePath);
    return ok;
  }
  catch (const std::exception& e)
  {
    LOG_WARN("Exception during metadata update: {}", e.what());
    return false;
  }
}

auto Parser::fillArtUrl(Metadata& meta) -> bool
{
  const auto cacheDir = ::utils::fs::getAppCacheArtPath();
  std::filesystem::create_directories(cacheDir.c_str());

  const std::string     hash   = std::to_string(std::hash<std::string>{}(meta.filePath.c_str()));
  std::filesystem::path outImg = cacheDir / (hash + ".jpg");

  auto* source = findSource(std::filesystem::path(meta.filePath).extension().string());
  if (!source)
    return false;

  // [TODO] Replace filesystem with DB to avoid cache explosion for large libraries
  if (std::filesystem::exists(outImg) ||
      source->extractThumbnail(meta.filePath.c_str(), outImg.c_str()))
  {
    meta.artUrl = ::utils::fs::toAbsFilePathUri(outImg);
    return true;
  }

  meta.artUrl.clear();
  return false;
}

} // namespace taglib
