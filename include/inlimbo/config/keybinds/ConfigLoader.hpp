#pragma once

#include <string>

namespace config::keybinds
{

class ConfigLoader
{
public:
  explicit ConfigLoader(std::string frontend);

  auto loadIntoRegistry(bool overwriteExisting = true) const -> void;

private:
  std::string m_frontend;
};

} // namespace config::keybinds
