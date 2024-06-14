#include "piconaut/http/http_server.h"

#include "http_server.h"

// cppcheck-suppress unknownMacro
PICONAUT_INNER_NAMESPACE(http)
MultiThreadedH2OServer::MultiThreadedH2OServer(const std::string& host,
                                               int port, int num_threads)
                : host_(host), num_threads_(num_threads), port_(port) {
  memset(&config_, 0, sizeof(config_));
  h2o_config_init(&config_);

  hostconf_ = h2o_config_register_host(
      &config_, h2o_iovec_init(host.c_str(), host.size()), port);

  if (hostconf_ == nullptr) {
    throw std::runtime_error("Failed to register host configuration");
  }
}

MultiThreadedH2OServer::~MultiThreadedH2OServer() {
  Stop();
}

void MultiThreadedH2OServer::Wait() {
  for (auto& t : threads_) {
    if (t.joinable())
      t.join();
  }
}
void MultiThreadedH2OServer::RegisterHandler(
    const std::string& path, std::shared_ptr<handlers::HandlerBase> handler) {
  h2o_pathconf_t* pathconf =
      h2o_config_register_path(hostconf_, path.c_str(), 0);

  auto h_handler = handlers::MakePiconautHandler(pathconf, handler);
  if (!h_handler)
    throw std::runtime_error("Error create handler for path:" + path);
  handlers_.push_back(handler);
  std::cout << "Registered handler for path: " << path << std::endl;
}

static void on_message(h2o_multithread_receiver_t* receiver,
                       h2o_linklist_t* messages) {
  // Handle messages between threads
}

static void AcceptConnection(h2o_socket_t* listener, const char* err) {
  std::cout << "Accepted connection" << std::endl;
  h2o_socket_t* sock;

  if (!listener)
    throw std::runtime_error("Null socket on_accept");

  if (err != NULL) {
    return;
  }

  if ((sock = h2o_evloop_socket_accept(listener)) == NULL)
    return;
  h2o_accept_ctx_t* ctx = (h2o_accept_ctx_t*)listener->data;
  h2o_accept(ctx, sock);
}

void MultiThreadedH2OServer::Start() {
  std::cout << "Server starting.." << std::endl;
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

  // Initialize the event loop and context
  contexts_.resize(num_threads_);
  loop_ = h2o_evloop_create();

  // queues_ = h2o_multithread_create_queue(loop_);
  // h2o_multithread_register_receiver(queues_, &receiver_, on_message);

  for (size_t i = 0; i < num_threads_; ++i) {
    h2o_loop_t* loop = h2o_evloop_create();
    h2o_context_init(&contexts_[i], loop_, &config_);
  }

  for (size_t i = 0; i < num_threads_; ++i) {
    h2o_socket_t* sock = h2o_evloop_socket_create(contexts_[i].loop, fd,
                                                  H2O_SOCKET_FLAG_DONT_READ);
    h2o_socket_read_start(sock, AcceptConnection);
  }

  for (size_t i = 0; i < num_threads_; ++i) {
    threads_.emplace_back(&MultiThreadedH2OServer::RunEventLoop, this, i);
  }

  std::cout << "Server running on " << host_ << ":" << port_ << " with "
            << num_threads_ << " threads" << std::endl;
}

void MultiThreadedH2OServer::Stop() {
  std::cout << "Server stopping..";
  if (queues_)
    h2o_multithread_destroy_queue(queues_);
  for (auto c : contexts_) {
    h2o_context_dispose(&c);
  }
  if (listener_)
    h2o_socket_close(listener_);
}

void MultiThreadedH2OServer::RunEventLoop(int thread_index) {
  std::cout << "Event Loop #" << std::to_string(thread_index) << std::endl;
  while (h2o_evloop_run(contexts_[thread_index].loop, INT32_MAX) == 0)
    ;
}

PICONAUT_INNER_END_NAMESPACE