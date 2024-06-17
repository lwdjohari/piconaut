#pragma once

#include <memory>
#include <mutex>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "piconaut/routers/route.h"
#include "piconaut/routers/router_node.h"
#include "piconaut/utils/string_utils.h"

PICONAUT_INNER_NAMESPACE(routers)
struct RouterMatchResult {
  const Route::HandlerFn* executor;
  const size_t* key;

  RouterMatchResult(const size_t* key,const Route::HandlerFn* fn)
                  : key(key), executor(fn) {}

  bool IsEmpty() const{
    if( !executor)
      return true;
    
    return false;
  }
};

/// @brief Router class for managing routing endpoint registration and
/// route-matching. Implementation conform to RFC-6570. This class is
/// thread-safe and designed for concurrent lock-free MatchRoute executions.
class Router {
 public:
  Router();

  // Delete copy constructor and copy assignment operator
  Router(const Router&) = delete;
  Router& operator=(const Router&) = delete;

  // Delete move constructor and move assignment operator
  Router(Router&&) = delete;
  Router& operator=(Router&&) = delete;

  void PrintRouterTree() const;
  void AddRoute(const size_t& key, const std::string& path,
                Route::HandlerFn handler);

  RouterMatchResult MatchRoute(
      const std::string& path,
      std::unordered_map<std::string, std::string>& params) const;

 private:
  std::unique_ptr<RouterNode> root_;
  mutable std::mutex mutex_;

  bool IsValidRoute(const std::string& path) const;
};
PICONAUT_INNER_END_NAMESPACE