#pragma once

#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <string>

namespace utils::unix
{

inline static auto demangleSymbol(const char* mangled) -> std::string
{
  int   status    = 0;
  char* demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);

  std::string result = (status == 0 && demangled) ? demangled : mangled;

  std::free(demangled);
  return result;
}

inline static auto prettifyDlError(const char* err) -> std::string
{
  if (!err)
    return {};

  std::string           msg(err);
  constexpr const char* needle = "undefined symbol: ";

  auto pos = msg.find(needle);
  if (pos == std::string::npos)
    return msg;

  pos += std::strlen(needle);
  std::string mangled = msg.substr(pos);

  return msg.substr(0, pos) + demangleSymbol(mangled.c_str());
}

} // namespace utils::unix
