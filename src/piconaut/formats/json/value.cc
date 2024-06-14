#include "piconaut/formats/json/value.h"

PICONAUT_INNER_NAMESPACE(formats)
namespace json {
Value::Value() : allocator_(document_.GetAllocator()), current_value_(&document_), is_empty_(true) {
  document_.SetObject();
}

Value::Value(const Value& other) : document_(), allocator_(document_.GetAllocator()), current_value_(&document_), is_empty_(other.is_empty_) {
  document_.CopyFrom(other.document_, allocator_);
}

Value& Value::operator=(const Value& other) {
  if (this != &other) {
    document_.CopyFrom(other.document_, allocator_);
    current_value_ = &document_;
    is_empty_ = other.is_empty_;
  }
  return *this;
}

Value& Value::operator[](const std::string& key) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  if (!document_.HasMember(key.c_str())) {
    rapidjson::Value json_key(key.c_str(), allocator_);
    document_.AddMember(json_key, rapidjson::Value(), allocator_);
  }

  current_value_ = &document_[key.c_str()];
  return *this;
}

void Value::operator=(const std::string& value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  if (current_value_) {
    *current_value_ = rapidjson::Value(value.c_str(), allocator_);
  }
}

void Value::operator=(const char* value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  if (current_value_) {
    *current_value_ = rapidjson::Value(value, allocator_);
  }
}

void Value::operator=(int value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  if (current_value_) {
    *current_value_ = rapidjson::Value(value);
  }
}

void Value::operator=(double value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  if (current_value_) {
    *current_value_ = rapidjson::Value(value);
  }
}

void Value::operator=(bool value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  if (current_value_) {
    *current_value_ = rapidjson::Value(value);
  }
}

void Value::operator=(const std::optional<std::string>& value) {
  if (value.has_value()) {
    *this = value.value();
  } else {
    *current_value_ = rapidjson::Value(rapidjson::kNullType);
  }
}

void Value::operator=(const std::optional<int>& value) {
  if (value.has_value()) {
    *this = value.value();
  } else {
    *current_value_ = rapidjson::Value(rapidjson::kNullType);
  }
}

void Value::operator=(const std::optional<double>& value) {
  if (value.has_value()) {
    *this = value.value();
  } else {
    *current_value_ = rapidjson::Value(rapidjson::kNullType);
  }
}

void Value::operator=(const std::optional<bool>& value) {
  if (value.has_value()) {
    *this = value.value();
  } else {
    *current_value_ = rapidjson::Value(rapidjson::kNullType);
  }
}

void Value::operator=(const std::vector<Value>& value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  rapidjson::Value array(rapidjson::kArrayType);
  for (const auto& item : value) {
    rapidjson::Value v(rapidjson::kObjectType);
    v.CopyFrom(item.document_, allocator_);
    array.PushBack(v, allocator_);
  }

  if (current_value_) {
    *current_value_ = array;
  }
}

void Value::operator=(const std::vector<std::string>& value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  rapidjson::Value array(rapidjson::kArrayType);
  for (const auto& item : value) {
    array.PushBack(rapidjson::Value(item.c_str(), allocator_), allocator_);
  }

  if (current_value_) {
    *current_value_ = array;
  }
}

void Value::operator=(const std::vector<int>& value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  rapidjson::Value array(rapidjson::kArrayType);
  for (int item : value) {
    array.PushBack(item, allocator_);
  }

  if (current_value_) {
    *current_value_ = array;
  }
}

void Value::operator=(const std::vector<double>& value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  rapidjson::Value array(rapidjson::kArrayType);
  for (double item : value) {
    array.PushBack(item, allocator_);
  }

  if (current_value_) {
    *current_value_ = array;
  }
}

void Value::operator=(const std::vector<bool>& value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  rapidjson::Value array(rapidjson::kArrayType);
  for (bool item : value) {
    array.PushBack(item, allocator_);
  }

  if (current_value_) {
    *current_value_ = array;
  }
}

template<typename T>
T Value::As() const {
  if (!current_value_) {
    throw std::runtime_error("Null JSON value");
  }

  if constexpr (std::is_same_v<T, std::string>) {
    if (!current_value_->IsString()) {
      throw std::runtime_error("JSON value is not a string");
    }
    return std::string(current_value_->GetString());
  } else if constexpr (std::is_same_v<T, int>) {
    if (!current_value_->IsInt()) {
      throw std::runtime_error("JSON value is not an integer");
    }
    return current_value_->GetInt();
  } else if constexpr (std::is_same_v<T, double>) {
    if (!current_value_->IsDouble()) {
      throw std::runtime_error("JSON value is not a double");
    }
    return current_value_->GetDouble();
  } else if constexpr (std::is_same_v<T, bool>) {
    if (!current_value_->IsBool()) {
      throw std::runtime_error("JSON value is not a boolean");
    }
    return current_value_->GetBool();
  } else if constexpr (std::is_same_v<T, std::optional<std::string>>) {
    if (current_value_->IsNull()) {
      return std::nullopt;
    }
    return As<std::string>();
  } else if constexpr (std::is_same_v<T, std::optional<int>>) {
    if (current_value_->IsNull()) {
      return std::nullopt;
    }
    return As<int>();
  } else if constexpr (std::is_same_v<T, std::optional<double>>) {
    if (current_value_->IsNull()) {
      return std::nullopt;
    }
    return As<double>();
  } else if constexpr (std::is_same_v<T, std::optional<bool>>) {
    if (current_value_->IsNull()) {
      return std::nullopt;
    }
    return As<bool>();
  } else if constexpr (std::is_same_v<T, std::vector<Value>>) {
    if (!current_value_->IsArray()) {
      throw std::runtime_error("JSON value is not an array");
    }
    std::vector<Value> result;
    for (auto& v : current_value_->GetArray()) {
      Value item;
      item.document_.CopyFrom(v, item.allocator_);
      result.push_back(item);
    }
    return result;
  } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
    if (!current_value_->IsArray()) {
      throw std::runtime_error("JSON value is not an array");
    }
    std::vector<std::string> result;
    for (auto& v : current_value_->GetArray()) {
      if (!v.IsString()) {
        throw std::runtime_error("JSON array element is not a string");
      }
      result.push_back(v.GetString());
    }
    return result;
  } else if constexpr (std::is_same_v<T, std::vector<int>>) {
    if (!current_value_->IsArray()) {
      throw std::runtime_error("JSON value is not an array");
    }
    std::vector<int> result;
    for (auto& v : current_value_->GetArray()) {
      if (!v.IsInt()) {
        throw std::runtime_error("JSON array element is not an integer");
      }
      result.push_back(v.GetInt());
    }
    return result;
  } else if constexpr (std::is_same_v<T, std::vector<double>>) {
    if (!current_value_->IsArray()) {
      throw std::runtime_error("JSON value is not an array");
    }
    std::vector<double> result;
    for (auto& v : current_value_->GetArray()) {
      if (!v.IsDouble()) {
        throw std::runtime_error("JSON array element is not a double");
      }
      result.push_back(v.GetDouble());
    }
    return result;
  } else if constexpr (std::is_same_v<T, std::vector<bool>>) {
    if (!current_value_->IsArray()) {
      throw std::runtime_error("JSON value is not an array");
    }
    std::vector<bool> result;
    for (auto& v : current_value_->GetArray()) {
      if (!v.IsBool()) {
        throw std::runtime_error("JSON array element is not a boolean");
      }
      result.push_back(v.GetBool());
    }
    return result;
  } else {
    throw std::runtime_error("Unsupported type for JSON deserialization");
  }
}

std::vector<uint8_t> Value::SerializeToBytes() const {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  if (!document_.Accept(writer)) {
    throw std::runtime_error("Failed to serialize JSON document.");
  }

  const char* json_str = buffer.GetString();
  return std::vector<uint8_t>(json_str, json_str + buffer.GetSize());
}
}  // namespace json

PICONAUT_INNER_END_NAMESPACE