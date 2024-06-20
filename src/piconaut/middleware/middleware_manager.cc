#include "piconaut/middleware/middleware_manager.h"

// cppcheck-suppress unknownMacro
PICONAUT_INNER_NAMESPACE(middleware)
void MiddlewareManager::Add(
    std::shared_ptr<MiddlewareBase> middleware) {
  middlewares_.push_back(middleware);
}

void MiddlewareManager::Handle(http::Request& req, http::Response& res) {
  std::function<void(size_t)> execute = [&](size_t index) {
    if (index < middlewares_.size()) {
      middlewares_[index]->Handle(req, res, [&]() { execute(index + 1); });
    }
  };
  execute(0);
}
PICONAUT_INNER_END_NAMESPACE