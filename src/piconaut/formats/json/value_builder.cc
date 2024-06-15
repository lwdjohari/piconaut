#include "piconaut/formats/json/value_builder.h"

PICONAUT_INNER_NAMESPACE(formats)
namespace json {
ValueBuilder::ValueBuilder()
                : allocator_(document_.GetAllocator()),
                  current_value_(&document_),
                  is_empty_(true),
                  current_type_(JsonValueType::kUnknown) {
  document_.SetObject();
}

ValueBuilder::ValueBuilder(const ValueBuilder& other)
                : document_(),
                  allocator_(document_.GetAllocator()),
                  current_value_(&document_),
                  is_empty_(other.is_empty_),
                  current_type_(JsonValueType::kUnknown) {
  document_.CopyFrom(other.document_, allocator_);
}

// ValueBuilder::~ValueBuilder() {}

rapidjson::Value* ValueBuilder::GetNodeValue() {
  if (value_nodes_.size() == 0) {
    return &document_;
  } else {
    return value_nodes_.back().value;
  }
}

ValueBuilder& ValueBuilder::operator=(const ValueBuilder& other) {
  if (this == &other)
    return *this;

  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  auto node = GetNodeValue();
  node->CopyFrom(other.document_, allocator_);
  current_type_ =
      static_cast<JsonValueType>(static_cast<int>(other.document_.GetType()));
  current_value_ = node;

  return *this;
}

// Get the json node or create the json node when not found.
//
// Note: For Json inner object creation.
// Must call ```.CreateJsonObject()```, otherwise you will get malformed json
// ```cpp
//
// // create json object with key "object" on root level
// json["object"].CreateJsonObject();
// json["object"]["oval"] = "ovalue";
//
// // create json object with key "inner" inside object node
// json["object"]["inner"].CreateJsonObject();
// json["object"]["inner"]["ival"] = "ivalue";
//
// ```
ValueBuilder& ValueBuilder::operator[](const std::string& key) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  // When found json["root"]["child"], it still executions on same object.
  // We have to emplace all key and pointer to key node
  // to make sure when assignment it pointing to correct node.
  // ```GetNodeValue()``` will determine which node to pointing.

  rapidjson::Value* node_value = GetNodeValue();
  if (!node_value->HasMember(key.c_str())) {
    rapidjson::Value json_key(key.c_str(), allocator_);
    node_value->AddMember(json_key, rapidjson::Value(), allocator_);
    current_type_ = JsonValueType::kUnknown;
    current_value_ = &(*node_value)[key.c_str()];
  } else {
    current_value_ = &(*node_value)[key.c_str()];
    current_type_ = static_cast<JsonValueType>(current_value_->GetType());
  }

  value_nodes_.emplace_back(std::string(key), current_value_);
  return *this;
}

// overload = operators

void ValueBuilder::CreateJsonObject() {
  if (!current_value_)
    throw std::runtime_error(
        "please call `json[\"key_name\"].CreateJsonObject()` before call "
        "CreateJsonObject on root");
  if (current_type_ == JsonValueType::kUnknown) {
    current_value_->SetObject();
    current_type_ = JsonValueType::kObjectType;
  } else {
    std::cerr << "Can't change the type initialized previously!";
  }

  value_nodes_.clear();
};

void ValueBuilder::operator=(const std::string& value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  if (!current_value_)
    return;

  if (current_type_ == JsonValueType::kUnknown ||
      current_type_ == JsonValueType::kStringType) {
    current_type_ = JsonValueType::kStringType;
    *current_value_ = rapidjson::Value(value.c_str(), allocator_);
  } else {
    std::cerr << "Can't change the type initialized previously!";
  }

  // must clear the nodes when do assignment
  value_nodes_.clear();
}

void ValueBuilder::operator=(unsigned int value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  if (!current_value_)
    return;

  if (current_type_ == JsonValueType::kUnknown ||
      current_type_ == JsonValueType::kNumberType) {
    current_type_ = JsonValueType::kNumberType;
    *current_value_ = rapidjson::Value(value);
  } else {
    std::cerr << "Can't change the type initialized previously!";
  }

  // must clear the nodes when do assignment
  value_nodes_.clear();
}

void ValueBuilder::operator=(int value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  if (!current_value_)
    return;

  if (current_type_ == JsonValueType::kUnknown ||
      current_type_ == JsonValueType::kNumberType) {
    current_type_ = JsonValueType::kNumberType;
    *current_value_ = rapidjson::Value(value);
  } else {
    std::cerr << "Can't change the type initialized previously!";
  }

  // must clear the nodes when do assignment
  value_nodes_.clear();
}

void ValueBuilder::operator=(double value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  if (!current_value_)
    return;

  if (current_type_ == JsonValueType::kUnknown ||
      current_type_ == JsonValueType::kNumberType) {
    current_type_ = JsonValueType::kNumberType;
    *current_value_ = rapidjson::Value(value);
  } else {
    std::cerr << "Can't change the type initialized previously!";
  }

  // must clear the nodes when do assignment
  value_nodes_.clear();
}

void ValueBuilder::operator=(float value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  if (!current_value_)
    return;

  if (current_type_ == JsonValueType::kUnknown ||
      current_type_ == JsonValueType::kNumberType) {
    current_type_ = JsonValueType::kNumberType;
    *current_value_ = rapidjson::Value(value);
  } else {
    std::cerr << "Can't change the type initialized previously!";
  }

  // must clear the nodes when do assignment
  value_nodes_.clear();
}

void ValueBuilder::operator=(bool value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  if (!current_value_)
    return;

  if (current_type_ == JsonValueType::kUnknown ||
      current_type_ == JsonValueType::kFalseType ||
      current_type_ == JsonValueType::kTrueType) {
    *current_value_ = rapidjson::Value(value);
  } else {
    std::cerr << "Can't change the type initialized previously!";
  }

  // must clear the nodes when do assignment
  value_nodes_.clear();
}

void ValueBuilder::operator=(const char* value) {
  if (is_empty_) {
    document_.SetObject();
    is_empty_ = false;
  }

  if (!current_value_)
    return;

  if (current_type_ == JsonValueType::kUnknown ||
      current_type_ == JsonValueType::kStringType) {
    *current_value_ = rapidjson::Value(value, allocator_);
  } else {
    std::cerr << "Can't change the type initialized previously!";
  }

  // must clear the nodes when do assignment
  value_nodes_.clear();
}

JsonBuffer ValueBuilder::SerializeToBytes() const {
  JsonBuffer buffer;
  buffer.buffer = std::move(std::make_shared<rapidjson::StringBuffer>());

  rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer.buffer);
  if (!document_.Accept(writer)) {
    throw std::runtime_error("Failed to serialize JSON document.");
  }

  buffer.data = buffer.buffer->GetString();
  buffer.size = buffer.buffer->GetSize();
  return __PCN_RETURN_MOVE(buffer);
}

JsonBuffer ValueBuilder::SerializePrettyToBytes() const {
  JsonBuffer buffer;
  buffer.buffer = std::move(std::make_shared<rapidjson::StringBuffer>());

  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(*buffer.buffer);
  if (!document_.Accept(writer)) {
    throw std::runtime_error("Failed to serialize JSON document.");
  }

  buffer.data = buffer.buffer->GetString();
  buffer.size = buffer.buffer->GetSize();
  return __PCN_RETURN_MOVE(buffer);
}
}  // namespace json
PICONAUT_INNER_END_NAMESPACE