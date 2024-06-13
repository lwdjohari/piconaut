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
  virtual void handle(const http::Request& req,
                      const http::Response& res) const = 0;

  static int HandlerCallback(h2o_handler_t* self, h2o_req_t* req) {
    piconaut_handler_t* pico_handler = (piconaut_handler_t*)self;
    std::shared_ptr<HandlerBase> handler = pico_handler->handler;
    http::Request request(req);
    http::Response response(req);
    handler->handle(request, response);
    return 0;
  }
};

inline h2o_handler_t* MakePiconautHandler(
    h2o_pathconf_t* pathconf, std::shared_ptr<HandlerBase> handler) {
  piconaut_handler_t* pico_handler =
      (piconaut_handler_t*)h2o_create_handler(pathconf, sizeof(*pico_handler));
  pico_handler->super.on_context_init = on_context_init;
  pico_handler->super.on_context_dispose = on_context_dispose;
  pico_handler->super.dispose = dispose;
  pico_handler->super.on_req = HandlerBase::HandlerCallback;
  pico_handler->handler = handler;
  return &pico_handler->super;
}

PICONAUT_INNER_END_NAMESPACE