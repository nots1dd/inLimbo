#pragma once

#include "InLimbo-Types.hpp"
#include "Logger.hpp"
#include "c/frontend_api.h"
#include "utils/fs/Paths.hpp"
#include "utils/string/SmallString.hpp"
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>

namespace frontend
{

struct PluginError : std::exception
{
  explicit PluginError(std::string msg) : m_msg(std::move(msg)) {}
  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

private:
  std::string m_msg;
};

struct PluginNotFoundError : PluginError
{
  using PluginError::PluginError;
};
struct PluginPermissionError : PluginError
{
  using PluginError::PluginError;
};
struct PluginLoadError : PluginError
{
  using PluginError::PluginError;
};
struct PluginAPIMismatchError : PluginError
{
  using PluginError::PluginError;
};
struct PluginInvalidBinaryError : PluginError
{
  using PluginError::PluginError;
};

using PluginName    = utils::string::SmallString;
using PluginNameStr = std::string;

class Plugin
{
public:
  explicit Plugin(const PluginName& name)
  {
    m_path = resolvePluginPath(name);
    validatePath(m_path.c_str());

    m_handleVPtr = dlopen(m_path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!m_handleVPtr)
      throw PluginLoadError(dlerror());

    auto* sym = reinterpret_cast<const InLimboFrontendAPI* (*)()>(
      dlsym(m_handleVPtr, "inlimbo_frontend_get_api"));

    if (!sym)
      throw PluginInvalidBinaryError("Missing inlimbo_frontend_get_api symbol");

    m_api = sym();
    validateAPI();
  }

  ~Plugin()
  {
    if (m_handleVPtr)
      dlclose(m_handleVPtr);
  }

  auto create(vptr songMap, vptr telemetry, vptr mpris) -> void*
  {
    return m_api->create(songMap, telemetry, mpris);
  }

  void run(vptr inst, vptr audio) { m_api->run(inst, audio); }

  void destroy(vptr inst) { m_api->destroy(inst); }

private:
  Path                      m_path;
  vptr                      m_handleVPtr = nullptr;
  const InLimboFrontendAPI* m_api        = nullptr;

  static auto pluginSearchPaths() -> Paths
  {
    Paths paths;

    const auto localPath = (utils::fs::getAppDataPath() / "frontends").string();

    LOG_DEBUG("Found local path: {}", localPath);

    // User data dir (XDG-compliant) - typically set to ~/.local/share
    paths.emplace_back(localPath);

    // since the project hasnt reached maturity yet,
    // ive refrained from adding system wide option.
    //
    // [TODO]: Add system wide plugin path (/usr/local/share and /usr/share; XDG_DATA_DIRS)

    return paths;
  }

  static auto resolvePluginPath(const PluginName& name) -> Path
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

    throw PluginNotFoundError("Frontend plugin not found: " + std::string(name.c_str()));
  }

  static void validatePath(PathCStr path)
  {
    struct stat st = {};
    if (stat(path, &st) != 0)
      throw PluginNotFoundError("Frontend not found");

    if (!S_ISREG(st.st_mode))
      throw PluginInvalidBinaryError("Frontend is not a regular file");

    if ((st.st_mode & S_IWOTH) || (st.st_mode & S_IWGRP))
      throw PluginPermissionError("Frontend is writable by others");

    if (st.st_uid != getuid())
      throw PluginPermissionError("Frontend is not owned by current user");
  }

  void validateAPI()
  {
    if (!m_api)
      throw PluginInvalidBinaryError("Null frontend API");

    if (m_api->abi_version != INLIMBO_FRONTEND_ABI_VERSION)
      throw PluginAPIMismatchError("Frontend ABI mismatch");

    if (m_api->struct_size != sizeof(InLimboFrontendAPI))
      throw PluginAPIMismatchError("Frontend struct size mismatch");

    if (!m_api->create || !m_api->run || !m_api->destroy)
      throw PluginInvalidBinaryError("Frontend API incomplete");
  }
};

} // namespace frontend
