#pragma once

#include <string>
#include <string_view>

#include <openssl/ssl.h>

namespace utils::unix::net
{

inline static constexpr const char* USER_AGENT             = "inLimbo/1.0 (Linux)";
inline static constexpr const char* HTTP_VERSION           = "HTTP/1.1";
inline static constexpr int         DEFAULT_HTTPS_PORT     = 443;
inline static constexpr const char* CARRIAGE_RETURN        = "\r\n";
inline static constexpr const char* DOUBLE_CARRIAGE_RETURN = "\r\n\r\n";

enum class ErrorCode
{
  None,
  DNSFailure,
  SocketFailure,
  ConnectFailure,
  TLSFailure,
  SendFailure,
  ReceiveFailure,
  HTTPError
};

struct Error
{
  ErrorCode   code = ErrorCode::None;
  std::string message;
};

template <typename T>
struct Result
{
  T     value{};
  Error error{};

  [[nodiscard]] auto ok() const -> bool { return error.code == ErrorCode::None; }
};

class SSLGlobalInit
{
public:
  SSLGlobalInit();
  static auto instance() -> SSLGlobalInit&;
};

class HTTPSClient final
{
public:
  HTTPSClient();

  auto get(std::string_view host, std::string_view path, int port = DEFAULT_HTTPS_PORT)
    -> Result<std::string>;

private:
  struct TLSSocket
  {
    int      fd  = -1;
    SSL*     ssl = nullptr;
    SSL_CTX* ctx = nullptr;
  };

  auto connect_tls(std::string_view host, int port) -> Result<TLSSocket>;

  auto https_get(TLSSocket& s, std::string_view host, std::string_view path) -> Result<std::string>;

  static void cleanup(TLSSocket& s);
};

} // namespace utils::unix::net
