#pragma once

#include "piconaut/handlers/handler_base.h"
#include "piconaut/macro.h"
#include "piconaut/routers/router.h"

PICONAUT_INNER_NAMESPACE(handlers)

class GlobalDispatcherHandler : public HandlerBase {
 public:
  GlobalDispatcherHandler() : router_() {}
  ~GlobalDispatcherHandler(){};

  routers::Router& Router() {
    return router_;
  }
  void Handle(const http::Request& req,
              const http::Response& res) const override {
    std::unordered_map<std::string, std::string> params;
    auto route_fn = router_.MatchRoute(req.GetPath(), params);
    if(!route_fn){
        //handle 404
        res.Send("Eror 404: Not found",404);
    }

    // execute the handler
    // TODO: change the route_fn callback signature
    // so we can get the instance of handler directly
  }

 private:
  routers::Router router_;
};

PICONAUT_INNER_END_NAMESPACE
