#include "utils/unix/net/HTTPSClient.hpp"
#include "Logger.hpp"

#include <sstream>

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <openssl/err.h>

namespace utils::unix::net
{

static void log_openssl_errors(const char* where)
{
  unsigned long err;

  while ((err = ERR_get_error()))
  {
    char buf[256];
    ERR_error_string_n(err, buf, sizeof(buf));
    LOG_ERROR("[TLS] {} → {}", where, buf);
  }
}

static void log_errno(const char* where)
{
  LOG_ERROR("[SYS] {} → errno={} ({})", where, errno, strerror(errno));
}

SSLGlobalInit::SSLGlobalInit()
{
  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
}

auto SSLGlobalInit::instance() -> SSLGlobalInit&
{
  static SSLGlobalInit inst;
  return inst;
}

HTTPSClient::HTTPSClient() { SSLGlobalInit::instance(); }

auto HTTPSClient::get(std::string_view host, std::string_view path, int port) -> Result<std::string>
{
  auto conn = connect_tls(host, port);
  if (!conn.ok())
    return {.value = {}, .error = conn.error};

  auto res = https_get(conn.value, host, path);

  cleanup(conn.value);

  return res;
}

auto HTTPSClient::connect_tls(std::string_view host, int port) -> Result<TLSSocket>
{
  TLSSocket out{};

  addrinfo hints{};
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  addrinfo* res;

  std::string portStr = std::to_string(port);

  if (getaddrinfo(host.data(), portStr.c_str(), &hints, &res) != 0)
  {
    log_errno("getaddrinfo");
    return {.value = {}, .error = {.code = ErrorCode::DNSFailure, .message = "DNS lookup failed"}};
  }

  int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if (sock < 0)
  {
    log_errno("socket");
    freeaddrinfo(res);
    return {.value = {},
            .error = {.code = ErrorCode::SocketFailure, .message = "Socket create failed"}};
  }

  if (connect(sock, res->ai_addr, res->ai_addrlen) < 0)
  {
    log_errno("connect");
    freeaddrinfo(res);
    close(sock);
    return {.value = {},
            .error = {.code = ErrorCode::ConnectFailure, .message = "TCP connect failed"}};
  }

  freeaddrinfo(res);

  SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
  if (!ctx)
  {
    log_openssl_errors("SSL_CTX_new");
    close(sock);
    return {.value = {}, .error = {.code = ErrorCode::TLSFailure, .message = "SSL_CTX_new failed"}};
  }

  SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);

  SSL_CTX_set_default_verify_paths(ctx);
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
  SSL_CTX_set_options(ctx, SSL_OP_IGNORE_UNEXPECTED_EOF);

  SSL* ssl = SSL_new(ctx);

  SSL_set_tlsext_host_name(ssl, host.data());
  SSL_set_fd(ssl, sock);

  if (SSL_connect(ssl) <= 0)
  {
    log_openssl_errors("SSL_connect");

    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sock);

    return {.value = {},
            .error = {.code = ErrorCode::TLSFailure, .message = "TLS handshake failed"}};
  }

  X509* cert = SSL_get_peer_certificate(ssl);
  if (!cert)
  {
    LOG_ERROR("[TLS] No peer certificate");

    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sock);

    return {.value = {},
            .error = {.code = ErrorCode::TLSFailure, .message = "No peer certificate"}};
  }

  X509_free(cert);

  if (SSL_get_verify_result(ssl) != X509_V_OK)
  {
    LOG_ERROR("[TLS] Certificate verification failed");

    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sock);

    return {.value = {}, .error = {.code = ErrorCode::TLSFailure, .message = "Cert verify failed"}};
  }

  out.fd  = sock;
  out.ssl = ssl;
  out.ctx = ctx;

  return {.value = out, .error = {}};
}

auto HTTPSClient::https_get(TLSSocket& s, std::string_view host, std::string_view path)
  -> Result<std::string>
{
  std::ostringstream req;
  req << "GET " << path << " " << HTTP_VERSION << CARRIAGE_RETURN;
  req << "Host: " << host << CARRIAGE_RETURN;
  req << "User-Agent: " << USER_AGENT << CARRIAGE_RETURN;
  req << "Accept: application/json" << CARRIAGE_RETURN;
  req << "Accept-Encoding: identity" << CARRIAGE_RETURN;
  req << "Connection: close" << DOUBLE_CARRIAGE_RETURN;

  auto reqStr = req.str();

  LOG_TRACE("[HTTPS] Sending request:");
  LOG_TRACE("[HTTPS] Host: {}", host);
  LOG_TRACE("[HTTPS] Path: {}", path);
  LOG_TRACE("[HTTPS] Request bytes: {}", reqStr.size());

  if (SSL_write(s.ssl, reqStr.c_str(), reqStr.size()) <= 0)
    return {.value = {}, .error = {.code = ErrorCode::SendFailure, .message = "SSL_write failed"}};

  std::string response;
  char        buf[4096];
  size_t      totalRead = 0;

  while (true)
  {
    int r = SSL_read(s.ssl, buf, sizeof(buf));

    if (r > 0)
    {
      response.append(buf, r);
      totalRead += r;
      continue;
    }

    int err = SSL_get_error(s.ssl, r);

    if (err == SSL_ERROR_ZERO_RETURN)
    {
      LOG_DEBUG("[TLS] clean shutdown");
      break;
    }
    else if (err == SSL_ERROR_SYSCALL)
    {
      if (errno != 0)
        log_errno("SSL_read syscall");
      else
        LOG_DEBUG("[TLS] EOF without close_notify");

      break;
    }
    else if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
    {
      continue;
    }
    else
    {
      log_openssl_errors("SSL_read");

      return {.value = {},
              .error = {.code = ErrorCode::ReceiveFailure, .message = "SSL_read failed"}};
    }
  }

  LOG_TRACE("[HTTPS] Received {} bytes total", totalRead);

  auto statusEnd = response.find(CARRIAGE_RETURN);
  if (statusEnd == std::string::npos)
    return {.value = {},
            .error = {.code = ErrorCode::HTTPError, .message = "Invalid HTTP response"}};

  auto statusLine = response.substr(0, statusEnd);

  LOG_TRACE("[HTTPS] Status: {}", statusLine);

  if (statusLine.find("200") == std::string::npos)
    return {.value = {}, .error = {.code = ErrorCode::HTTPError, .message = statusLine}};

  auto bodyPos = response.find(DOUBLE_CARRIAGE_RETURN);
  if (bodyPos == std::string::npos)
    return {.value = {}, .error = {.code = ErrorCode::HTTPError, .message = "No HTTP body"}};

  auto body = response.substr(bodyPos + 4);

  return {.value = body, .error = {}};
}

void HTTPSClient::cleanup(TLSSocket& s)
{
  if (s.ssl)
  {
    SSL_shutdown(s.ssl);
    SSL_free(s.ssl);
  }

  if (s.ctx)
    SSL_CTX_free(s.ctx);

  if (s.fd >= 0)
    close(s.fd);
}

} // namespace utils::unix::net
