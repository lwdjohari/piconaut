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
#include "piconaut/routers/router.h"
#include "piconaut/middleware/middleware_manager.h"
#include "piconaut/handlers/global_dispatcher_handler.h"

PICONAUT_INNER_NAMESPACE(http)

class H2OServer {
 public:
  H2OServer(const std::string& host, int port,
            const std::string& server_name = "piconaut/0.2.1[h2o/2.2.5]");
  ~H2OServer();
  void SetConfig(const Config& config);
  const Config& GetConfig() const;
  bool SetSSL();
  
  void RegisterMiddleware(std::shared_ptr<middleware::MiddlewareBase> middleware);
  void RegisterHandler(const std::string& path,
                       std::shared_ptr<handlers::HandlerBase> handler);
  void Start();
  void Stop();

 private:
 void RegisterGlobalHandler(std::shared_ptr<handlers::HandlerBase> handler);
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
  std::string server_name_;
  std::shared_ptr<handlers::GlobalDispatcherHandler> routers_;
};
PICONAUT_INNER_END_NAMESPACE