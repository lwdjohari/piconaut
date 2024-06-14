
#include "piconaut/http/http_single_server.h"

// cppcheck-suppress unknownMacro
PICONAUT_INNER_NAMESPACE(http)

H2OServer::H2OServer(const std::string& host, int port)
                : host_(host), port_(port) {
  memset(&config_, 0, sizeof(config_));
  h2o_config_init(&config_);

  hostconf_ = h2o_config_register_host(
      &config_, h2o_iovec_init(host.c_str(), host.size()), port);

  if (hostconf_ == nullptr) {
    throw std::runtime_error("Failed to register host configuration");
  }
}

H2OServer::~H2OServer() {
  Stop();
  std::cout << "Server stopping..";

  h2o_socket_close(listeners_);
  h2o_context_dispose(&contexts_);
  h2o_config_dispose(&config_);
}

void H2OServer::RegisterHandler(
    const std::string& path, std::shared_ptr<handlers::HandlerBase> handler) {
  h2o_pathconf_t* pathconf =
      h2o_config_register_path(hostconf_, path.c_str(), 0);

  auto h_handler = handlers::MakePiconautHandler(pathconf, handler);
  if (!h_handler)
    throw std::runtime_error("Error create handler for path:" + path);
  handlers_.push_back(handler);
  std::cout << "Registered handler for path: " << path << std::endl;
}

void H2OServer::AcceptConnection(h2o_socket_t* listener, const char* err) {
  std::cout << "Accepted connection" << std::endl;
  h2o_socket_t* sock;

  if (err != NULL) {
    return;
  }

  if ((sock = h2o_evloop_socket_accept(listener)) == NULL)
    return;
  h2o_accept_ctx_t* ctx = (h2o_accept_ctx_t*)listener->data;
  h2o_accept(ctx, sock);
}

void H2OServer::Start() {
  std::cout << "Server starting.." << std::endl;

  memset(&accept_ctx_, 0, sizeof(accept_ctx_));
  accept_ctx_.hosts = config_.hosts;

  int fd;
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, host_.c_str(), &addr.sin_addr);
  addr.sin_port = htons(port_);

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("failed to create socket");
    throw std::runtime_error("Failed to create socket");
  }

  if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
    perror("failed to bind to the specified host and port");
    close(fd);
    throw std::runtime_error("Failed to bind to the specified host and port");
  }

  if (listen(fd, SOMAXCONN) != 0) {
    perror("failed to listen on socket");
    close(fd);
    throw std::runtime_error("Failed to listen on socket");
  }

  loops_ = h2o_evloop_create();
  if (loops_ == nullptr)
    throw std::runtime_error("Failed to create evloop");
  h2o_context_init(&contexts_, loops_, &config_);

  listeners_ = h2o_evloop_socket_create(loops_, fd, H2O_SOCKET_FLAG_DONT_READ);
  if (listeners_ == nullptr) {
    perror("failed to create listener socket");
    close(fd);
    throw std::runtime_error("Failed to create listener socket");
  }

  accept_ctx_.ctx = &contexts_;
  listeners_->data = &accept_ctx_;

  // auto accept_func = [](h2o_socket_t* sock, const char* err) {
  //   h2o_accept_ctx_t* ctx = (h2o_accept_ctx_t*)sock->data;
  //   std::cout << "Accepted connection"  << std::endl;
  //   h2o_accept(ctx, sock);
  // };

  h2o_socket_read_start(listeners_, AcceptConnection);
  std::cout << "Server running on " << host_ << ":" << port_ << std::endl;
  RunEventLoop();
}



void H2OServer::Stop() {
  
}

void H2OServer::RunEventLoop() {
  while (h2o_evloop_run(contexts_.loop, INT32_MAX) == 0)
    ;
}
PICONAUT_INNER_END_NAMESPACE