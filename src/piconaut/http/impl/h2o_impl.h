#pragma once

#include <h2o.h>

#include <memory>
#include <string>
#include <atomic>
#include <vector>
#include "piconaut/http/declare.h"
#include "piconaut/http/config.h"

// #include "piconaut/http/middleware_manager.h"
#include "piconaut/macro.h"

PICONAUT_INNER_NAMESPACE(http)
namespace impl {

struct H2OServerImpl;

class ServerH2O {
 public:
  ServerH2O(const std::string& host, const std::string& port, bool is_ssl);
  ~ServerH2O();

  void Configure(const Config& config);
  void AddRoute(const std::string& path, std::shared_ptr<HandlerBase> handler,
                MiddlewareManager& middleware_manager);
  void Start();
  void Stop();

 private:
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};
}  // namespace impl
PICONAUT_INNER_END_NAMESPACE