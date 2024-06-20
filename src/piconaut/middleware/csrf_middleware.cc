#include "piconaut/middleware/csrf_middleware.h"

// cppcheck-suppress unknownMacro
PICONAUT_INNER_NAMESPACE(middleware)

CsrfMiddleware::CsrfMiddleware() {
  csrf_token_ = GenerateToken();
}

void CsrfMiddleware::Handle(http::Request& req, http::Response& res,
                            std::function<void()> next) {
  if (req.Method() == "POST") {
    std::string token = req.GetHeader("X-CSRF-Token");
    if (!ValidateToken(token)) {
      res.Status(403);
      res.Send("Forbidden: CSRF token invalid");
      return;
    }
  }
  res.AddHeader("X-CSRF-Token", csrf_token_);
  next();
}

std::string CsrfMiddleware::GenerateToken() {
  std::mt19937_64 rng(
      std::chrono::system_clock::now().time_since_epoch().count());
  std::uniform_int_distribution<std::mt19937_64::result_type> dist(0,
                                                                   UINT64_MAX);
  return std::to_string(dist(rng));
}

bool CsrfMiddleware::ValidateToken(const std::string& token) {
  return token == csrf_token_;
}

PICONAUT_INNER_END_NAMESPACE