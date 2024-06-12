#include "piconaut/http/impl/h2o_impl.h"

PICONAUT_INNER_NAMESPACE(http)

namespace impl {

struct H2OServerImpl {
  h2o_hostconf_t* hostconf;
  h2o_accept_ctx_t accept_ctx;
  std::vector<std::shared_ptr<HandlerBase>> handlers;
  std::string host;
  int port;
  bool is_ssl;
  h2o_globalconf_t config;
  h2o_context_t context;
  h2o_compress_args_t compress_args;
  uv_loop_t loop;
  MiddlewareManagerPtr middleware_manager;
  ConfigPtr server_config;
  std::atomic<bool> server_running{false};

  H2OServerImpl(const std::string& host, const int& port, bool is_ssl)
                  : host(host), port(port), is_ssl(is_ssl) {
    this->host = std::string(host);
    this->port = int(port);
    this->is_ssl = is_ssl;

    h2o_config_init(&config);
    uv_loop_init(&loop);
    h2o_context_init(&context, &loop, &config);
    hostconf = h2o_config_register_host(
        &config, h2o_iovec_init(this->host.c_str(), this->host.size()),
        this->port);
  }

  ~H2OServerImpl() {
    h2o_context_dispose(&context);
    h2o_config_dispose(&config);
    uv_loop_close(&loop);
  }

  void SignalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
      server_running = false;
    }
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

    // Enable DoS protection
    if (config.RateLimit() > 0 && config.ConnectionLimit() > 0) {
      h2o_access_log_filehandle_t* fh = h2o_access_log_open_handle("/dev/null", NULL, H2O_LOGCONF_ESCAPE_APACHE);
      h2o_access_log_register(hostconf, fh);
      h2o_limits_init(&this->config.limits, config.RateLimit(), config.ConnectionLimit());
    }

    // Enable HTTPS
    if (is_ssl) {
      SSL_CTX* ssl_ctx = SSL_CTX_new(TLS_method());
      SSL_CTX_use_certificate_file(ssl_ctx, config.CertFile().c_str(),
                                   SSL_FILETYPE_PEM);
      SSL_CTX_use_PrivateKey_file(ssl_ctx, config.KeyFile().c_str(),
                                  SSL_FILETYPE_PEM);
      hostconf->ssl = ssl_ctx;
    }

    // Enable CORS
    if (!config.Cors().empty()) {
      h2o_headers_command_t headers_commands[] = {
          {H2O_HEADERS_CMD_ADD,
           {H2O_TOKEN_ACCESS_CONTROL_ALLOW_ORIGIN,
            {config.Cors().c_str(), config.Cors().size()}}},
          {H2O_HEADERS_CMD_ADD,
           {H2O_TOKEN_ACCESS_CONTROL_ALLOW_METHODS,
            H2O_STRLIT("GET, POST, OPTIONS")}},
          {H2O_HEADERS_CMD_ADD,
           {H2O_TOKEN_ACCESS_CONTROL_ALLOW_HEADERS,
            H2O_STRLIT("Content-Type")}},
          {}};
      h2o_headers_register(&this->config, headers_commands);
    }
  }

  void AddRoute(const std::string& path, std::shared_ptr<HandlerBase> handler,
                MiddlewareManager& middleware_manager) {
    h2o_pathconf_t* pathconf =
        h2o_config_register_path(hostconf, path.c_str(), 0);
    h2o_handler_t* handler_fn =
        h2o_create_handler(pathconf, sizeof(*handler_fn));
    handler_fn->on_req = [](h2o_handler_t* self, h2o_req_t* req) -> int {
      auto server_impl = static_cast<H2OServerImpl*>(self->data);
      Request request(req);
      Response response(req);
      server_impl->middleware_manager.Handle(request, response);
      return 0;
    };
    handler_fn->data = this;
    handlers.push_back(handler);
  }

  // void Start() {
  //   signal(SIGINT, SignalHandler);
  //   signal(SIGTERM, SignalHandler);

  //   struct sockaddr_storage ss;
  //   socklen_t ss_len = h2o_sockaddr_set(&ss, host.c_str(), port.c_str(),
  //                                       H2O_SOCKADDR_FLAG_IPV4);
  //   auto listener = h2o_evloop_socket_create(
  //       context.loop, (struct sockaddr*)&ss, ss_len,
  //       H2O_SOCKET_FLAG_DONT_READ);
  //   h2o_accept_setup(listener, &accept_ctx);

  //   std::cout << "Server running on " << host << ":" << port << std::endl;

  //   while (server_running && h2o_evloop_run(context.loop, INT32_MAX) == 0)
  //     ;

  //   std::cout << "Server shutting down..." << std::endl;

  //   // Graceful shutdown
  //   h2o_evloop_socket_close(listener);
  // }

  // void Stop() {
  //   server_running = false;
  // }
};

// ServerH2O::ServerH2O(const std::string& host, const std::string& port,
//                      bool is_ssl)
//                 : pimpl_(std::make_unique<Impl>(host, port, is_ssl)) {}

// ServerH2O::~ServerH2O() = default;

// void ServerH2O::Configure(const Config& config) {
//   pimpl_->Configure(config);
// }

// void ServerH2O::AddRoute(const std::string& path,
//                          std::shared_ptr<HandlerBase> handler,
//                          MiddlewareManager& middleware_manager) {
//   pimpl_->AddRoute(path, handler, middleware_manager);
// }

// void ServerH2O::Start() {
//   pimpl_->Start();
// }

// void ServerH2O::Stop() {
//   pimpl_->Stop();
// }

}  // namespace impl

PICONAUT_INNER_END_NAMESPACE