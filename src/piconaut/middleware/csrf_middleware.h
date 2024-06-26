#pragma once

#include <chrono>
#include <ctime>
#include <functional>
#include <random>
#include <string>

#include "piconaut/macro.h"
#include "piconaut/middleware/middleware_base.h"

PICONAUT_INNER_NAMESPACE(middleware)

class CsrfMiddleware : public MiddlewareBase {
 public:
  CsrfMiddleware();
  void Handle(http::Request& req, http::Response& res,
              std::function<void()> next) override;

 private:
  std::string GenerateToken();
  bool ValidateToken(const std::string& token);

  std::string csrf_token_;
};

PICONAUT_INNER_END_NAMESPACE