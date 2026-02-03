#pragma once

#include "InLimbo-Types.hpp"
#include "c/frontend_api.h"
#include "utils/string/SmallString.hpp"

#include <exception>
#include <string>

using PluginName    = utils::string::SmallString;
using PluginNameStr = std::string;

namespace frontend
{

enum class PluginErrorCode
{
  NotFound,
  PermissionDenied,
  InvalidBinary,
  LoadFailure,
  SymbolMissing,
  APIMismatch,
  InternalError,
};

struct PluginError : std::exception
{
  PluginError(PluginErrorCode code, std::string summary, std::string details = {}, Path path = {});

  [[nodiscard]] auto what() const noexcept -> const char* override;
  [[nodiscard]] auto code() const noexcept -> PluginErrorCode;

private:
  PluginErrorCode m_code;
  std::string     m_summary;
  std::string     m_details;
  Path            m_path;

  std::string m_formatted;

  static auto codeToString(PluginErrorCode c) -> const char*;
  static auto indent(const std::string& s, int spaces) -> std::string;
  void        format();
};

class Plugin
{
public:
  explicit Plugin(const PluginName& name);
  ~Plugin();

  auto create(vptr songMap, vptr telemetry, vptr mpris) -> void*;
  void run(vptr inst, vptr audio);
  void destroy(vptr inst);

private:
  Path                      m_path;
  vptr                      m_handleVPtr = nullptr;
  const InLimboFrontendAPI* m_api        = nullptr;

  static auto pluginSearchPaths() -> Paths;
  static auto dumpSearchPaths() -> Path;
  static auto resolvePluginPath(const PluginName& name) -> Path;

  static void validatePath(const Path& path);
  void        validateAPI();
};

} // namespace frontend
