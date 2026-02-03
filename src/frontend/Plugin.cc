#include "frontend/Plugin.hpp"
#include "Logger.hpp"
#include "utils/fs/Paths.hpp"
#include "utils/unix/Symbols.hpp"

#include <cerrno>
#include <cstring>
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>

namespace frontend
{

PluginError::PluginError(PluginErrorCode code, std::string summary, std::string details, Path path)
    : m_code(code), m_summary(std::move(summary)), m_details(std::move(details)),
      m_path(std::move(path))
{
  format();
}

auto PluginError::what() const noexcept -> const char* { return m_formatted.c_str(); }

auto PluginError::code() const noexcept -> PluginErrorCode { return m_code; }

auto PluginError::codeToString(PluginErrorCode c) -> const char*
{
  switch (c)
  {
    case PluginErrorCode::NotFound:
      return "NotFound";
    case PluginErrorCode::PermissionDenied:
      return "PermissionDenied";
    case PluginErrorCode::InvalidBinary:
      return "InvalidBinary";
    case PluginErrorCode::LoadFailure:
      return "LoadFailure";
    case PluginErrorCode::SymbolMissing:
      return "SymbolMissing";
    case PluginErrorCode::APIMismatch:
      return "APIMismatch";
    default:
      return "InternalError";
  }
}

auto PluginError::indent(const std::string& s, int spaces) -> std::string
{
  std::string pad(spaces, ' ');
  std::string out;

  size_t pos = 0;
  while (true)
  {
    size_t next = s.find('\n', pos);
    out += pad + s.substr(pos, next - pos) + "\n";
    if (next == std::string::npos)
      break;
    pos = next + 1;
  }
  return out;
}

void PluginError::format()
{
  m_formatted = "\n[Frontend Plugin Error Dump]\n";
  m_formatted += "  Code    : ";
  m_formatted += codeToString(m_code);
  m_formatted += "\n";
  m_formatted += "  Summary : " + m_summary + "\n";

  if (!m_path.empty())
  {
    m_formatted += "  Path    : ";
    m_formatted += m_path;
    m_formatted += "\n";
  }

  if (!m_details.empty())
  {
    m_formatted += "  Details :\n";
    m_formatted += indent(m_details, 4);
  }
}

Plugin::Plugin(const PluginName& name)
{
  m_path = resolvePluginPath(name);
  validatePath(m_path);

  m_handleVPtr = dlopen(m_path.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (!m_handleVPtr)
  {
    throw PluginError(PluginErrorCode::LoadFailure, "Failed to load frontend plugin",
                      utils::unix::prettifyDlError(dlerror()), m_path);
  }

  auto* sym = reinterpret_cast<const InLimboFrontendAPI* (*)()>(
    dlsym(m_handleVPtr, "inlimbo_frontend_get_api"));

  if (!sym)
  {
    throw PluginError(PluginErrorCode::SymbolMissing,
                      "Missing required symbol: inlimbo_frontend_get_api",
                      utils::unix::prettifyDlError(dlerror()), m_path);
  }

  m_api = sym();
  validateAPI();
}

Plugin::~Plugin()
{
  if (m_handleVPtr)
    dlclose(m_handleVPtr);
}

auto Plugin::create(vptr songMap, vptr telemetry, vptr mpris) -> void*
{
  return m_api->create(songMap, telemetry, mpris);
}

void Plugin::run(vptr inst, vptr audio) { m_api->run(inst, audio); }

void Plugin::destroy(vptr inst) { m_api->destroy(inst); }

auto Plugin::pluginSearchPaths() -> Paths
{
  Paths paths;

  auto localPath = (utils::fs::getAppDataPath() / "frontends").string();
  LOG_DEBUG("Frontend plugin path: {}", localPath);

  paths.emplace_back(localPath);
  return paths;
}

auto Plugin::dumpSearchPaths() -> Path
{
  Path out;
  for (const auto& p : pluginSearchPaths())
  {
    out += "  - ";
    out += p;
    out += "\n";
  }
  return out;
}

auto Plugin::resolvePluginPath(const PluginName& name) -> Path
{
  utils::string::SmallString filename("libinlimbo-frontend-");
  filename += name;
  filename += ".so";

  for (const auto& dir : pluginSearchPaths())
  {
    Path full(dir);
    full += "/";
    full += filename;

    if (access(full.c_str(), R_OK) == 0)
      return full;
  }

  throw PluginError(PluginErrorCode::NotFound, "Frontend plugin not found",
                    "Plugin name  : " + std::string(name.c_str()) +
                      "\n"
                      "Search paths :\n" +
                      dumpSearchPaths().c_str());
}

void Plugin::validatePath(const Path& path)
{
  struct stat st{};
  if (stat(path.c_str(), &st) != 0)
  {
    throw PluginError(PluginErrorCode::NotFound, "Frontend binary does not exist",
                      std::string("stat() failed: ") + strerror(errno), path);
  }

  if (!S_ISREG(st.st_mode))
  {
    throw PluginError(PluginErrorCode::InvalidBinary, "Frontend is not a regular file", {}, path);
  }

  if ((st.st_mode & S_IWOTH) || (st.st_mode & S_IWGRP))
  {
    throw PluginError(PluginErrorCode::PermissionDenied,
                      "Frontend binary is writable by group/others", "Fix with: chmod go-w <file>",
                      path);
  }

  if (st.st_uid != getuid())
  {
    throw PluginError(PluginErrorCode::PermissionDenied,
                      "Frontend binary is not owned by current user", {}, path);
  }
}

void Plugin::validateAPI()
{
  if (!m_api)
  {
    throw PluginError(PluginErrorCode::InvalidBinary, "Frontend API pointer is null", {}, m_path);
  }

  if (m_api->abi_version != INLIMBO_FRONTEND_ABI_VERSION)
  {
    throw PluginError(PluginErrorCode::APIMismatch, "Frontend ABI mismatch",
                      "Expected ABI version : " + std::to_string(INLIMBO_FRONTEND_ABI_VERSION) +
                        "\n"
                        "Plugin ABI version   : " +
                        std::to_string(m_api->abi_version),
                      m_path);
  }

  if (m_api->struct_size != sizeof(InLimboFrontendAPI))
  {
    throw PluginError(PluginErrorCode::APIMismatch, "Frontend struct size mismatch",
                      "Expected size : " + std::to_string(sizeof(InLimboFrontendAPI)) +
                        "\n"
                        "Plugin size   : " +
                        std::to_string(m_api->struct_size),
                      m_path);
  }

  if (!m_api->create || !m_api->run || !m_api->destroy)
  {
    throw PluginError(PluginErrorCode::InvalidBinary, "Frontend API incomplete",
                      "One or more required function pointers are null", m_path);
  }
}

} // namespace frontend
