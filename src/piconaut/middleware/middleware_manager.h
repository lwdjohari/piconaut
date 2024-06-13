#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "piconaut/macro.h"
#include "piconaut/middleware/middleware_base.h"
PICONAUT_INNER_NAMESPACE(middleware)

class MiddlewareManager {
 public:
  void add_middleware(std::shared_ptr<MiddlewareBase> middleware);
  void handle(http::Request& req, http::Response& res);

 private:
  std::vector<std::shared_ptr<MiddlewareBase>> middlewares_;
};
PICONAUT_INNER_END_NAMESPACE