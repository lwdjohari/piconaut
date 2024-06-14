#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "piconaut/http/request.h"
#include "piconaut/http/response.h"
#include "piconaut_handler_t.h"

PICONAUT_INNER_NAMESPACE(handlers)

class HandlerBase {
 public:
  virtual ~HandlerBase() = default;
  virtual void Handle(const http::Request& req,
                      const http::Response& res) const = 0;

  static int HandlerCallback(h2o_handler_t* self, h2o_req_t* req) {
    if (!self || !req)
      return 1;

    std::cout << "Callback received:" << std::string(req->path_normalized.base, req->path_normalized.len) << std::endl;
    
    piconaut_handler_t* pico_handler = (piconaut_handler_t*)self;
    HandlerBase* handler = static_cast<HandlerBase*>(pico_handler->handler);
    http::Request request(req);
    http::Response response(req);
    handler->Handle(request, response);
    return 0;
  }
};

inline h2o_handler_t* MakePiconautHandler(
    h2o_pathconf_t* pathconf, std::shared_ptr<HandlerBase> handler) {
  piconaut_handler_t* pico_handler;

  pico_handler =
      (piconaut_handler_t*)h2o_create_handler(pathconf, sizeof(*pico_handler));
  // pico_handler->handler_context.on_context_init = on_context_init;
  // pico_handler->handler_context.on_context_dispose = on_context_dispose;
  pico_handler->handler_context.dispose = dispose;
  pico_handler->handler_context.on_req = HandlerBase::HandlerCallback;
  pico_handler->handler = static_cast<void*>(handler.get());
  return &pico_handler->handler_context;
}

PICONAUT_INNER_END_NAMESPACE