#pragma once

#include <arpa/inet.h>
#include <h2o.h>
#include <unistd.h>

#include <iostream>
#include <thread>

#include "piconaut/handlers/handler_base.h"
#include "piconaut/http/config.h"
#include "piconaut/http/declare.h"
#include "piconaut/macro.h"
// #include "piconaut/http/impl/h2o_impl.h"

PICONAUT_INNER_NAMESPACE(http)

class H2OServer {
 public:
  H2OServer(const std::string& host, int port);
  ~H2OServer();
  void RegisterHandler(const std::string& path,
                        std::shared_ptr<handlers::HandlerBase> handler);
  void Start();
  void Stop();

 private:
  void RunEventLoop();
  static void AcceptConnection(h2o_socket_t* sock, const char* err);

  std::string host_;
  int port_;
  h2o_globalconf_t config_;
  h2o_context_t contexts_;
  h2o_evloop_t* loops_;
  h2o_socket_t* listeners_;
  std::vector<std::shared_ptr<handlers::HandlerBase>> handlers_;
  h2o_hostconf_t* hostconf_;
  h2o_accept_ctx_t accept_ctx_;
};
PICONAUT_INNER_END_NAMESPACE