#pragma once

#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "piconaut/macro.h"

PICONAUT_INNER_NAMESPACE(routers)
class Route {
 public:
  using HandlerFn =
      std::function<void(const std::unordered_map<std::string, std::string>&)>;

  Route(const std::string& path, HandlerFn handler);

  const std::string& Path() const;
  const HandlerFn& Handler() const;

 private:
  std::string path_;
  HandlerFn handler_;
};

PICONAUT_INNER_END_NAMESPACE