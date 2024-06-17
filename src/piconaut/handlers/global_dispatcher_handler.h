#pragma once

#include <mutex>
#include <unordered_map>
#include <utility>

#include "piconaut/handlers/handler_base.h"
#include "piconaut/macro.h"
#include "piconaut/routers/router.h"
PICONAUT_INNER_NAMESPACE(handlers)

class GlobalDispatcherHandler : public HandlerBase {
 public:
  GlobalDispatcherHandler() : router_(), routes_(), hasher_(), mutex_() {}
  ~GlobalDispatcherHandler(){};

  void RegisterRouteHandler(const std::string& path,
                            std::shared_ptr<HandlerBase> handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto key = hasher_(path);
    router_.AddRoute(key, path, GlobalDispatcherHandler::Dispatcher);
    routes_.emplace(key, std::make_shared<routers::Route>(key, path, handler));
  }

  routers::Router& Router() {
    return router_;
  }

  void __HandleImpl(const http::Request& req,
                    const http::Response& res) const override {
    std::unordered_map<std::string, std::string> params;
    auto route_result = router_.MatchRoute(req.GetPath(), params);
    if (route_result.IsEmpty()) {
      // handle 404
      res.Send("Eror 404: Not found", 404);
      return;
    }

    // It shall be cleaned by route match route
    // No need to check contains
    // If it's crashed, it means our logic in Match Route is flawed
    std::cout << "Dispatch to: " << req.GetPath() << std::endl; 
    auto fn = route_result.executor;

    auto req_handler = routes_.at(*route_result.key)->RequestHandler();
    (*fn)(req, res, req_handler, params);
  }

  void HandleRequest(const http::Request& req, const http::Response& res,
                     const std::unordered_map<std::string, std::string>& params)
      const override {
        
        // Nothing to-do in global dispatcher
      };

 private:
  routers::Router router_;
  std::unordered_map<size_t, std::shared_ptr<routers::Route>> routes_;
  std::hash<std::string> hasher_;
  std::mutex mutex_;

  static void Dispatcher(
      const http::Request& request, const http::Response& response,
      std::shared_ptr<handlers::HandlerBase> handler,
      const std::unordered_map<std::string, std::string>& params) {
    if (!handler)
      return;

    handler->HandleRequest(request, response, params);
  }
};

PICONAUT_INNER_END_NAMESPACE
