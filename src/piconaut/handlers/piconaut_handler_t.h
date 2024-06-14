#pragma once
#include <h2o.h>

#include <memory>

#include "piconaut/macro.h"

PICONAUT_INNER_NAMESPACE(handlers)
class HandlerBase;

// Piconaut h2o handler binding
// This handler carry around pointer to the c++ corespending HandlerBase
struct piconaut_handler_t {
  h2o_handler_t handler_context;
  void* handler;  // std::shared_ptr<HandlerBase>
};

// inline void on_context_init(h2o_handler_t* self, h2o_context_t* ctx) {
//   // No context-specific initialization needed
// }

// inline void on_context_dispose(h2o_handler_t* self, h2o_context_t* ctx) {
//   // No context-specific disposal needed
// }

inline void dispose(h2o_handler_t* self) {
  piconaut_handler_t* custom_handler = (piconaut_handler_t*)self;
  //custom_handler->handler.reset();
  free(custom_handler);
}

PICONAUT_INNER_END_NAMESPACE