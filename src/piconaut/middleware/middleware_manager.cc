#include "piconaut/middleware/middleware_manager.h"

// cppcheck-suppress unknownMacro
PICONAUT_INNER_NAMESPACE(middleware)
void MiddlewareManager::add_middleware(
    std::shared_ptr<MiddlewareBase> middleware) {
  middlewares_.push_back(middleware);
}

void MiddlewareManager::handle(http::Request& req, http::Response& res) {
  std::function<void(size_t)> execute = [&](size_t index) {
    if (index < middlewares_.size()) {
      middlewares_[index]->handle(req, res, [&]() { execute(index + 1); });
    }
  };
  execute(0);
}
PICONAUT_INNER_END_NAMESPACE