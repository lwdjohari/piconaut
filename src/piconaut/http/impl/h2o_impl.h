#pragma once

#include <h2o.h>

#include <functional>
#include <openssl/ssl.h>

#include <atomic>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "piconaut/http/config.h"
#include "piconaut/http/declare.h"

// #include "piconaut/http/middleware_manager.h"
#include "piconaut/macro.h"

PICONAUT_INNER_NAMESPACE(http)
namespace impl {

void SignalHandler(int signal);

class H2OServerImpl {
 private:
  h2o_hostconf_t* h2o_hostconf_;
  h2o_accept_ctx_t h2o_accept_context_;
  h2o_globalconf_t h2o_config_;
  h2o_context_t h2o_context_;
  h2o_compress_args_t h2o_compress_args_;
  h2o_access_log_filehandle_t* log_handle_;
  uv_loop_t uv_loop_;
  std::vector<std::shared_ptr<HandlerBase>> handlers_;
  MiddlewareManagerPtr middlewares_;
  ConfigPtr server_config_;
  std::atomic<bool> server_running_;
  std::string host_;
  int port_;
  uint16_t worker_number_;
  bool is_ssl_;
  uv_tcp_t uv_tcp_;
  std::shared_ptr<std::function<void(uv_stream_t*, int)>> uv_conn_callback_;

  int CreateListener(void) {
    struct sockaddr_in addr;
    int r;

    uv_tcp_init(h2o_context_.loop, &uv_tcp_);
    uv_tcp_.data = new std::shared_ptr<std::function<void(uv_stream_t*, int)>>(
        uv_conn_callback_);
    uv_ip4_addr("127.0.0.1", 7890, &addr);

    r = uv_listen((uv_stream_t*)&uv_tcp_, 128, OnNewConnection);
    if (r) {
      std::cerr << "Listen error: " << uv_strerror(r) << std::endl;
      uv_close((uv_handle_t*)&uv_tcp_, NULL);
    } else {
      std::cout << "Listening on " << host_ << ":" << port_ << std::endl;
    }

    return r;
  }

  void ConfigureCors(const Config& config) {
    // Enable CORS
    if (!config.Cors().empty()) {
      h2o_iovec_t allow_origin =
          h2o_iovec_init(H2O_STRLIT("access-control-allow-origin"));
      h2o_iovec_t allow_methods =
          h2o_iovec_init(H2O_STRLIT("access-control-allow-methods"));
      h2o_iovec_t allow_headers =
          h2o_iovec_init(H2O_STRLIT("access-control-allow-headers"));
      h2o_iovec_t allow_methods_val =
          h2o_iovec_init(H2O_STRLIT("GET, POST, OPTIONS"));
      h2o_iovec_t allow_headers_val =
          h2o_iovec_init(H2O_STRLIT("Content-Type"));

      h2o_headers_command_t headers_commands[] = {
          {H2O_HEADERS_CMD_ADD, &allow_origin,
           h2o_iovec_init(config.Cors().c_str(), config.Cors().size())},
          {H2O_HEADERS_CMD_ADD, &allow_methods, allow_methods_val},
          {H2O_HEADERS_CMD_ADD, &allow_headers, allow_headers_val},
          {}  // Terminator for the commands array
      };

      h2o_headers_register(&h2o_hostconf_->fallback_path, headers_commands);
    }
  }

  void ConfigureCompression(const Config& config) {
    // Set compression
    this->h2o_compress_args_ = {0};

    if (config.Compression() == CompressionType::GZIP) {
      // Set quality for GZIP
      h2o_compress_args_.gzip.quality = 1;
      h2o_compress_register(&h2o_hostconf_->fallback_path, &h2o_compress_args_);
    } else if (config.Compression() == CompressionType::BROTLI) {
      // Set quality for Brotli
      h2o_compress_args_.brotli.quality = 1;
      h2o_compress_register(&h2o_hostconf_->fallback_path, &h2o_compress_args_);
    }
  }

  void ConfigureHttpOptions(const Config& config) {
    // Set HTTP version
    if (config.HttpVersion() == HttpVersionMode::HTTP1_1) {
      this->h2o_config_.http1.req_timeout = config.Http1RequestTimeout();
      this->h2o_config_.http1.upgrade_to_http2 = 0;
    } else if (config.HttpVersion() == HttpVersionMode::HTTP2) {
      this->h2o_config_.http1.upgrade_to_http2 = 0;
      this->h2o_config_.http2.idle_timeout = config.Http2IdleTimeout();
      this->h2o_config_.http2.graceful_shutdown_timeout =
          config.Http2GracefulShutdownTimeout();
      this->h2o_config_.http2.max_concurrent_requests_per_connection =
          config.Http2MaxConcurrentRequestsPerConnection();
      this->h2o_config_.http2.max_streams_for_priority =
          config.Http2MaxStreamsForPriority();
    } else if (config.HttpVersion() == HttpVersionMode::HTTP2_AUTO_FALLBACK) {
      this->h2o_config_.http2.idle_timeout = config.Http2IdleTimeout();
      this->h2o_config_.http2.graceful_shutdown_timeout =
          config.Http2GracefulShutdownTimeout();
      this->h2o_config_.http2.max_concurrent_requests_per_connection =
          config.Http2MaxConcurrentRequestsPerConnection();
      this->h2o_config_.http2.max_streams_for_priority =
          config.Http2MaxStreamsForPriority();
      this->h2o_config_.http1.upgrade_to_http2 = 1;
    }
  }

  void ConfigureXssSecurity(const Config& config){
    // Enable XSS protection
    if (config.XssProtection()) {
      h2o_iovec_t header_name = h2o_iovec_init(H2O_STRLIT("x-xss-protection"));
      h2o_iovec_t header_value = h2o_iovec_init(H2O_STRLIT("1; mode=block"));

      h2o_headers_command_t headers_commands[] = {
          {H2O_HEADERS_CMD_ADD, &header_name, header_value},
          {}  // IMPORTANT!! Terminator for the commands array
      };
      h2o_headers_register(&h2o_hostconf_->fallback_path, headers_commands);
    }
  }

  void ConfigureCsrfSecurity(const Config& config){
     // Enable CSRF protection
    if (config.CsrfProtection()) {
      // middlewares_.AddMiddleware(std::make_shared<CsrfMiddleware>());
    }
  }

 public:
  H2OServerImpl(const std::string& host, const int& port, bool is_ssl,
                const uint16_t& worker_number)
                  : server_running_(),
                    host_(std::string(host)),
                    port_(int(port)),
                    worker_number_(uint16_t(worker_number)),
                    is_ssl_(is_ssl) {
    // Set the number of worker threads in the libuv thread pool

    h2o_config_ = {0};
    h2o_config_init(&h2o_config_);
    h2o_hostconf_ = h2o_config_register_host(
        &h2o_config_, h2o_iovec_init(this->host_.c_str(), this->host_.size()),
        this->port_);

    // Redirect the h2o log to the Piconaut log handler
    // So we can redirect to NvLog
    // auto log_callback = [](const char* log_line, size_t log_line_len) {
    //   std::cout.write(log_line, log_line_len);
    //   std::cout << std::endl;
    // };

    // create_custom_logger(&hostconf->fallback_path, log_callback);

    uv_conn_callback_ =
        std::make_shared<std::function<void(uv_stream_t*, int)>>(
            [this](uv_stream_t* listener, int status) {
              uv_tcp_t conn;
              h2o_socket_t* sock;

              if (status != 0)
                return;

              uv_tcp_init(listener->loop, &conn);

              if (uv_accept(listener, (uv_stream_t*)&conn) != 0) {
                uv_close((uv_handle_t*)&conn, (uv_close_cb)free);
                return;
              }

              sock =
                  h2o_uv_socket_create((uv_stream_t*)&conn, (uv_close_cb)free);
              h2o_accept(&h2o_accept_context_, sock);
            });
  }

  ~H2OServerImpl() {
    h2o_context_dispose(&h2o_context_);
    h2o_config_dispose(&h2o_config_);
    uv_loop_close(&uv_loop_);
  }

  void Configure(const Config& config) {
    server_config_ = std::make_shared<Config>(config);

    ConfigureHttpOptions(*server_config_);
    ConfigureCompression(*server_config_);
    ConfigureCors(*server_config_);
    ConfigureXssSecurity(*server_config_);
    ConfigureCsrfSecurity(*server_config_);
  }

  void AddRoute(const std::string& path, std::shared_ptr<HandlerBase> handler,
                MiddlewareManager& middlewares) {
    h2o_pathconf_t* pathconf =
        h2o_config_register_path(h2o_hostconf_, path.c_str(), 0);
    h2o_handler_t* handler_fn =
        h2o_create_handler(pathconf, sizeof(*handler_fn));
    handler_fn->on_req = [](h2o_handler_t* self, h2o_req_t* req) -> int {
      // auto server_impl = static_cast<H2OServerImpl*>(self->data);
      // Request request(req);
      // Response response(req);
      // server_impl->middlewares_.Handle(request, response);
      return 0;
    };
    // handler_fn->data = this;
    handlers_.push_back(handler);
  }

  void Start() {
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    h2o_access_log_filehandle_t* logfh = h2o_access_log_open_handle(
        "/dev/stdout", NULL, H2O_LOGCONF_ESCAPE_APACHE);

    uv_loop_init(&uv_loop_);
    h2o_context_init(&h2o_context_, &uv_loop_, &h2o_config_);

    uv_loop_init(&uv_loop_);

    h2o_accept_context_.ctx = &h2o_context_;
    h2o_accept_context_.hosts = h2o_config_.hosts;

    if (CreateListener() != 0) {
      std::cerr << "failed to listen to 127.0.0.1:7890:%s\n";
    }

    uv_run((uv_loop_t*)&h2o_context_.loop, UV_RUN_DEFAULT);
  }

  static void OnNewConnection(uv_stream_t* server, int status) {
    // Retrieve the lambda from the data field and call it
    auto callback = *reinterpret_cast<
        std::shared_ptr<std::function<void(uv_stream_t*, int)>>*>(server->data);
    (*callback)(server, status);
  }

  void Stop() {
    // send signal
  }
};
}  // namespace impl
PICONAUT_INNER_END_NAMESPACE