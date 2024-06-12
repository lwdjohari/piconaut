#pragma once

#include <h2o.h>

#include <functional>
// #include <h2o/openssl.h>
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
  h2o_hostconf_t* hostconf;
  h2o_accept_ctx_t accept_ctx;
  h2o_globalconf_t config;
  h2o_context_t context;
  h2o_compress_args_t compress_args;
  h2o_access_log_filehandle_t* log_handle_;
  uv_loop_t loop;
  std::vector<std::shared_ptr<HandlerBase>> handlers;
  MiddlewareManagerPtr middleware_manager;
  ConfigPtr server_config;
  std::atomic<bool> server_running;
  std::string host;
  int port;
  uint16_t worker_number;
  bool is_ssl;
  uv_tcp_t server_;
  std::shared_ptr<std::function<void(uv_stream_t*, int)>> connection_callback_;

  int CreateListener(void) {
    struct sockaddr_in addr;
    int r;

    uv_tcp_init(context.loop, &server_);
    server_.data = new std::shared_ptr<std::function<void(uv_stream_t*, int)>>(
        connection_callback_);
    uv_ip4_addr("127.0.0.1", 7890, &addr);

    r = uv_listen((uv_stream_t*)&server_, 128, OnNewConnection);
    if (r) {
      std::cerr << "Listen error: " << uv_strerror(r) << std::endl;
      uv_close((uv_handle_t*)&server_, NULL);
    } else {
      std::cout << "Listening on " << host << ":" << port << std::endl;
    }

    return r;
  }

 public:
  H2OServerImpl(const std::string& host, const int& port, bool is_ssl,
                const uint16_t& worker_number)
                  : server_running(),
                    host(std::string(host)),
                    port(int(port)),
                    worker_number(uint16_t(worker_number)),
                    is_ssl(is_ssl) {
    // Set the number of worker threads in the libuv thread pool

    h2o_config_init(&config);
    hostconf = h2o_config_register_host(
        &config, h2o_iovec_init(this->host.c_str(), this->host.size()),
        this->port);

    // Redirect the h2o log to the Piconaut log handler
    // So we can redirect to NvLog
    auto log_callback = [](const char* log_line, size_t log_line_len) {
      std::cout.write(log_line, log_line_len);
      std::cout << std::endl;
    };

    //create_custom_logger(&hostconf->fallback_path, log_callback);

    connection_callback_ =
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
              h2o_accept(&accept_ctx, sock);
            });
  }

  ~H2OServerImpl() {
    h2o_context_dispose(&context);
    h2o_config_dispose(&config);
    uv_loop_close(&loop);
  }

  void Configure(const Config& config) {
    server_config = std::make_shared<Config>(config);

    // Set HTTP version
    if (config.HttpVersion() == HttpVersionMode::HTTP1_1) {
      this->config.http1.req_timeout = config.Http1RequestTimeout();
      this->config.http1.upgrade_to_http2 = 0;
    } else if (config.HttpVersion() == HttpVersionMode::HTTP2) {
      this->config.http1.upgrade_to_http2 = 0;
      this->config.http2.idle_timeout = config.Http2IdleTimeout();
      this->config.http2.graceful_shutdown_timeout =
          config.Http2GracefulShutdownTimeout();
      this->config.http2.max_concurrent_requests_per_connection =
          config.Http2MaxConcurrentRequestsPerConnection();
      this->config.http2.max_streams_for_priority =
          config.Http2MaxStreamsForPriority();
    } else if (config.HttpVersion() == HttpVersionMode::HTTP2_AUTO_FALLBACK) {
      this->config.http2.idle_timeout = config.Http2IdleTimeout();
      this->config.http2.graceful_shutdown_timeout =
          config.Http2GracefulShutdownTimeout();
      this->config.http2.max_concurrent_requests_per_connection =
          config.Http2MaxConcurrentRequestsPerConnection();
      this->config.http2.max_streams_for_priority =
          config.Http2MaxStreamsForPriority();
      this->config.http1.upgrade_to_http2 = 1;
    }

    // Set compression
    this->compress_args = {0};

    if (config.Compression() == CompressionType::GZIP) {
      compress_args.gzip.quality = 1;  // Set quality for GZIP
      h2o_compress_register(&hostconf->fallback_path, &compress_args);
    } else if (config.Compression() == CompressionType::BROTLI) {
      compress_args.brotli.quality = 1;  // Set quality for Brotli
      h2o_compress_register(&hostconf->fallback_path, &compress_args);
    }

    // Enable XSS protection
    if (config.XssProtection()) {
      h2o_iovec_t header_name = h2o_iovec_init(H2O_STRLIT("x-xss-protection"));
      h2o_iovec_t header_value = h2o_iovec_init(H2O_STRLIT("1; mode=block"));

      h2o_headers_command_t headers_commands[] = {
          {H2O_HEADERS_CMD_ADD, &header_name, header_value},
          {}  // IMPORTANT!! Terminator for the commands array
      };
      h2o_headers_register(&hostconf->fallback_path, headers_commands);
    }

    // Enable CSRF protection
    if (config.CsrfProtection()) {
      // middleware_manager.AddMiddleware(std::make_shared<CsrfMiddleware>());
    }

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

      h2o_headers_register(&hostconf->fallback_path, headers_commands);
    }
  }

  void AddRoute(const std::string& path, std::shared_ptr<HandlerBase> handler,
                MiddlewareManager& middleware_manager) {
    h2o_pathconf_t* pathconf =
        h2o_config_register_path(hostconf, path.c_str(), 0);
    h2o_handler_t* handler_fn =
        h2o_create_handler(pathconf, sizeof(*handler_fn));
    handler_fn->on_req = [](h2o_handler_t* self, h2o_req_t* req) -> int {
      // auto server_impl = static_cast<H2OServerImpl*>(self->data);
      // Request request(req);
      // Response response(req);
      // server_impl->middleware_manager.Handle(request, response);
      return 0;
    };
    // handler_fn->data = this;
    handlers.push_back(handler);
  }

  void Start() {
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    h2o_access_log_filehandle_t* logfh = h2o_access_log_open_handle(
        "/dev/stdout", NULL, H2O_LOGCONF_ESCAPE_APACHE);

    
    uv_loop_init(&loop);
    h2o_context_init(&context, &loop, &config);

    uv_loop_init(&loop);

    accept_ctx.ctx = &context;
    accept_ctx.hosts = config.hosts;

    if (CreateListener() != 0) {
      std::cerr << "failed to listen to 127.0.0.1:7890:%s\n";
    }

    uv_run((uv_loop_t*)&context.loop, UV_RUN_DEFAULT);
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