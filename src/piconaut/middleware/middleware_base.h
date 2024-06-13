#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "piconaut/http/request.h"
#include "piconaut/http/response.h"
#include "piconaut/macro.h"

PICONAUT_INNER_NAMESPACE(middleware)

class MiddlewareBase {
 public:
  virtual ~MiddlewareBase() = default;
  virtual void handle(http::Request& req, http::Response& res,
                      std::function<void()> next) = 0;
};



PICONAUT_INNER_END_NAMESPACE