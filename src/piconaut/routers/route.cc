#include "piconaut/routers/route.h"

// cppcheck-suppress unknownMacro
PICONAUT_INNER_NAMESPACE(routers)

Route::Route(const size_t key, const std::string& path,
             std::shared_ptr<handlers::HandlerBase> req_handler)
                : key_(key),
                  path_(std::string(path)),
                  req_handler_(req_handler)
                   {}

const std::size_t& Route::Key() const {
  return key_;
}

const std::string& Route::Path() const {
  return path_;
}

const std::shared_ptr<handlers::HandlerBase> Route::RequestHandler() const {
  return req_handler_;
}


PICONAUT_INNER_END_NAMESPACE
