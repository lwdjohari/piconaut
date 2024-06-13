#include "piconaut/http/http_server.h"

// cppcheck-suppress unknownMacro
PICONAUT_INNER_NAMESPACE(http)
MultiThreadedH2OServer::MultiThreadedH2OServer(const std::string& host,
                                               int num_threads, int port)
                : host_(host),
                  num_threads_(num_threads),
                  port_(port),
                  listener_(nullptr) {
  h2o_config_init(&config_);
  contexts_.resize(num_threads_);
  loops_.resize(num_threads_);
  queues_.resize(num_threads_);

  for (int i = 0; i < num_threads_; ++i) {
    loops_[i] = h2o_evloop_create();
    h2o_context_init(&contexts_[i], loops_[i], &config_);
    queues_[i] = h2o_multithread_create_queue(loops_[i]);
  }

  hostconf_ = h2o_config_register_host(
      &config_, h2o_iovec_init(host.c_str(), host.size()), 65535);

  memset(&accept_ctx_, 0, sizeof(accept_ctx_));
  accept_ctx_.hosts = config_.hosts;
}

MultiThreadedH2OServer::~MultiThreadedH2OServer() {
  stop();
  for (auto& ctx : contexts_) {
    h2o_context_dispose(&ctx);
  }
  h2o_config_dispose(&config_);
}

void MultiThreadedH2OServer::register_handler(
    const std::string& path, std::shared_ptr<handlers::HandlerBase> handler) {
  h2o_pathconf_t* pathconf =
      h2o_config_register_path(hostconf_, path.c_str(), 0);
  handlers::MakePiconautHandler(pathconf, handler);
}

void MultiThreadedH2OServer::start() {
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

  listener_ = h2o_evloop_socket_create(contexts_[0].loop, fd,
                                       H2O_SOCKET_FLAG_DONT_READ);
  if (listener_ == nullptr) {
    perror("failed to create listener socket");
    close(fd);
    throw std::runtime_error("Failed to create listener socket");
  }

  for (int i = 0; i < num_threads_; ++i) {
    accept_ctx_.ctx = &contexts_[i];
    listener_->data = &accept_ctx_;

    auto accept_func = [](h2o_socket_t* sock, const char* err) {
      h2o_accept_ctx_t* ctx = (h2o_accept_ctx_t*)sock->data;
      h2o_accept(ctx, sock);
    };

    h2o_socket_read_start(listener_, accept_func);

    threads_.emplace_back(&MultiThreadedH2OServer::run_event_loop, this, i);
  }

  std::cout << "Server running on " << host_ << ":" << port_ << " with "
            << num_threads_ << " threads" << std::endl;
}

void MultiThreadedH2OServer::stop() {
  for (auto& thread : threads_) {
    if (thread.joinable()) {
      thread.join();
    }
  }
  if (listener_) {
    h2o_socket_close(listener_);
  }
}

void MultiThreadedH2OServer::run_event_loop(int thread_index) {
  while (h2o_evloop_run(contexts_[thread_index].loop, INT32_MAX) == 0)
    ;
}

PICONAUT_INNER_END_NAMESPACE