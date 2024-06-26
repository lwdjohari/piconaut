#pragma once

#include <arpa/inet.h>
#include <h2o.h>
#include <unistd.h>

#include <iostream>
#include <thread>

#include "piconaut/handlers/handler_base.h"
#include "piconaut/http/declare.h"
#include "piconaut/macro.h"
#include "piconaut/http/config.h"
// #include "piconaut/http/impl/h2o_impl.h"

PICONAUT_INNER_NAMESPACE(http)

class MultiThreadedH2OServer {
 public:
  MultiThreadedH2OServer(const std::string& host, int port, int num_threads = 4);
  ~MultiThreadedH2OServer();
  void Wait();
  void RegisterHandler(const std::string& path,
                        std::shared_ptr<handlers::HandlerBase> handler);
  void Start();
  void Stop();

 private:
  void RunEventLoop(int thread_index);

  std::string host_;
  int num_threads_;
  int port_;
  h2o_globalconf_t config_;
  std::vector<h2o_context_t> contexts_;
  h2o_loop_t* loop_;
  h2o_multithread_receiver_t receiver_;
  h2o_multithread_queue_t* queues_;
   h2o_socket_t* listener_;
  std::vector<std::thread> threads_;
  std::vector<std::shared_ptr<handlers::HandlerBase>> handlers_;
  h2o_hostconf_t* hostconf_;
  std::vector<h2o_accept_ctx_t> accept_ctx_;
};
PICONAUT_INNER_END_NAMESPACE