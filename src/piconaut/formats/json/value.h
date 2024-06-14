#pragma once


#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <optional>

#include "piconaut/macro.h"

PICONAUT_INNER_NAMESPACE(formats)
namespace json {



class Value {
 public:
  Value();
  Value(const Value& other);             // Copy constructor
  Value& operator=(const Value& other);  // Copy assignment operator
  Value& operator[](const std::string& key);

  void operator=(const std::string& value);
  void operator=(const char* value);
  void operator=(int value);
  void operator=(double value);
  void operator=(bool value);
  void operator=(const std::optional<std::string>& value);
  void operator=(const std::optional<int>& value);
  void operator=(const std::optional<double>& value);
  void operator=(const std::optional<bool>& value);
  void operator=(const std::vector<Value>& value);
  void operator=(const std::vector<std::string>& value);
  void operator=(const std::vector<int>& value);
  void operator=(const std::vector<double>& value);
  void operator=(const std::vector<bool>& value);

  template <typename T>
  T As() const;

  std::vector<uint8_t> SerializeToBytes() const;

 private:
  rapidjson::Document document_;
  rapidjson::Value* current_value_;
  rapidjson::Document::AllocatorType& allocator_;
  bool is_empty_;

  void Init();
};



}  // namespace json
PICONAUT_INNER_END_NAMESPACE