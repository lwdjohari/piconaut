#include "piconaut/routers/route.h"

// cppcheck-suppress unknownMacro
PICONAUT_INNER_NAMESPACE(routers)

Route::Route(const std::string& path, HandlerFn handler)
                : path_(std::string(path)), handler_(std::move(handler)) {}

const std::string& Route::Path() const {
  return path_;
}
const Route::HandlerFn& Route::Handler() const {
  return handler_;
}

PICONAUT_INNER_END_NAMESPACE
