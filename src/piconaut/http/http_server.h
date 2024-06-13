#pragma once

#include <arpa/inet.h>
#include <h2o.h>
#include <unistd.h>

#include <iostream>
#include <thread>

#include "piconaut/handlers/handler_base.h"
#include "piconaut/http/declare.h"
#include "piconaut/macro.h"
// #include "piconaut/http/impl/h2o_impl.h"

PICONAUT_INNER_NAMESPACE(http)

class MultiThreadedH2OServer {
 public:
  MultiThreadedH2OServer(const std::string& host, int num_threads, int port);
  ~MultiThreadedH2OServer();
  void register_handler(const std::string& path,
                        std::shared_ptr<handlers::HandlerBase> handler);
  void start();
  void stop();

 private:
  void run_event_loop(int thread_index);

  std::string host_;
  int num_threads_;
  int port_;
  h2o_globalconf_t config_;
  std::vector<h2o_context_t> contexts_;
  std::vector<h2o_evloop_t*> loops_;
  std::vector<h2o_multithread_queue_t*> queues_;
  std::vector<std::thread> threads_;
  h2o_hostconf_t* hostconf_;
  h2o_socket_t* listener_;
  h2o_accept_ctx_t accept_ctx_;
};
PICONAUT_INNER_END_NAMESPACE