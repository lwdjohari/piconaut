#pragma once


#include <arpa/inet.h>
#include <h2o.h>
#include <h2o/socket.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "piconaut/handlers/handler_base.h"
#include "piconaut/http/config.h"
#include "piconaut/http/declare.h"
#include "piconaut/macro.h"
#include "piconaut/middleware/middleware_manager.h"

PICONAUT_INNER_NAMESPACE(http)
namespace impl {

class H2OServerImpl {
 private:
  h2o_hostconf_t* h2o_hostconf_;
  h2o_accept_ctx_t h2o_accept_context_;
  h2o_globalconf_t h2o_config_;
  std::vector<h2o_context_t> h2o_contexts_;
  h2o_compress_args_t h2o_compress_args_;
  h2o_access_log_filehandle_t* log_handle_;
  std::vector<std::shared_ptr<HandlerBase>> handlers_;
  MiddlewareManagerPtr middlewares_;
  ConfigPtr server_config_;
  std::atomic<bool> server_running_;
  std::string host_;
  int port_;
  uint16_t worker_number_;
  bool is_ssl_;
  // std::shared_ptr<std::function<void(uv_stream_t*, int)>> uv_conn_callback_;

  // Function to handle connections
  static void OnAccept(h2o_socket_t* sock, const char* err) {
    if (err != nullptr) {
      h2o_socket_close(sock);
      return;
    }
    h2o_socket_read_start(sock, [](h2o_socket_t* sock, const char* err) {
      if (err != nullptr) {
        h2o_socket_close(sock);
        return;
      }
      h2o_req_t* req = static_cast<h2o_req_t*>(h2o_mem_alloc(sizeof(*req)));
      h2o_accept_ctx_t* ctx = static_cast<h2o_accept_ctx_t*>(sock->data);
      h2o_accept(ctx, sock);
    });
  }

  // Thread function to run the event loop
  static void* RunLoop(void* arg) {
    h2o_context_t* ctx = static_cast<h2o_context_t*>(arg);
    h2o_evloop_t* loop = ctx->loop;
    while (h2o_evloop_run(loop, INT32_MAX) == 0)
      ;
    return nullptr;
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

  void ConfigureXssSecurity(const Config& config) {
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

  void ConfigureCsrfSecurity(const Config& config) {
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
  }

  ~H2OServerImpl() {
    // Cleanup
    for (auto& ctx : h2o_contexts_) {
      h2o_context_dispose(&ctx);
    }
    h2o_config_dispose(&h2o_config_);
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
    server_running_ = true;

    // Initialize contexts and run them in separate threads
    h2o_contexts_.resize(worker_number_);
    pthread_t threads[worker_number_];
    for (size_t i = 0; i < worker_number_; ++i) {
      h2o_evloop_t* loop = h2o_evloop_create();
      h2o_context_init(&h2o_contexts_[i], loop, &h2o_config_);

      // Set up the accept context for each loop
      h2o_accept_context_.ctx = &h2o_contexts_[i];
      h2o_accept_context_.hosts = h2o_config_.hosts;

      pthread_create(&threads[i], nullptr, RunLoop, &h2o_contexts_[i]);
    }

    // Create and bind the listening socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, SOMAXCONN);

    // Accept connections and assign to worker threads
    for (size_t i = 0; i < worker_number_; ++i) {
      h2o_socket_t* sock = h2o_evloop_socket_create(
          h2o_contexts_[i].loop, listen_fd, H2O_SOCKET_FLAG_DONT_READ);
      sock->data = &h2o_accept_context_;
      h2o_socket_read_start(sock, OnAccept);
    }

    // Wait for all threads to finish
    for (size_t i = 0; i < worker_number_; ++i) {
      pthread_join(threads[i], nullptr);
    }
  }

  void Stop() {
    server_running_ = false;
  }
};
}  // namespace impl
PICONAUT_INNER_END_NAMESPACE