#include "telemetry/IDs.hpp"
#include "cereal/archives/binary.hpp"
#include <fstream>

namespace telemetry
{

auto Registry::save(const std::string& path) const -> bool
{
  std::ofstream os(path, std::ios::binary);
  if (!os)
    return false;

  cereal::BinaryOutputArchive ar(os);
  ar(*this);
  return true;
}

auto Registry::load(const std::string& path) -> bool
{
  std::ifstream is(path, std::ios::binary);
  if (!is)
    return false;

  cereal::BinaryInputArchive ar(is);
  ar(*this);
  return true;
}

} // namespace telemetry
