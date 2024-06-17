#pragma once
#include <iostream>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "piconaut/macro.h"
#include "piconaut/routers/declare.h"
#include "piconaut/routers/route.h"

PICONAUT_INNER_NAMESPACE(routers)

/// @brief Radix Tree Node
struct RouterNode {
  std::unordered_map<std::string, std::unique_ptr<RouterNode>> children;
  Route::HandlerFn handler = nullptr;
  NodeType type = NodeType::kStatic;
  std::string param_name;
  std::regex param_regex;
  size_t key;

  void PrintNode(const std::string& prefix = "") const {
    for (const auto& child : children) {
      std::cout << prefix << child.first;
      if (child.second->type == NodeType::kParameter) {
        std::cout << " (param: " << child.second->param_name << ")";
      }
      std::cout << std::endl;
      child.second->PrintNode(prefix + "  ");
    }
  }
};

PICONAUT_INNER_END_NAMESPACE