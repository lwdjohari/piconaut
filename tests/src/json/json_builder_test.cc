#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>

#include "piconaut/formats/json/value_builder.h"

using namespace piconaut;
TEST_CASE("[JsonBuilder] Single Level Test", "[JsonBuilder]") {
  auto answer = "{\"key1\":\"value1\",\"key2\":\"value2\",\"key3\":\"value3\"}";
  formats::json::ValueBuilder json;
  json["key1"] = "value1";
  json["key2"] = "value2";
  json["key3"] = "value3";

  auto val = json.SerializeToBytes();
  auto str_json = val.ToString();

  std::cout << "Json:\n" << str_json << std::endl;
  REQUIRE(answer == str_json);
}

TEST_CASE("[JsonBuilder] Multi Level Test", "[JsonBuilder]") {
  auto answer = "{\"key0\":\"value0\",\"object\":{\"key1\":\"value1\",\"key2\":"
                "\"value2\",\"inner\":{\"status\":1}},\"other\":true}";
  formats::json::ValueBuilder json;
  json["key0"] = "value0";

  json["object"].CreateJsonObject();
  json["object"]["key1"] = "value1";
  json["object"]["key2"] = "value2";

  json["object"]["inner"].CreateJsonObject();
  json["object"]["inner"]["status"] = 1;

  json["other"] = true;

  auto val = json.SerializeToBytes();
  auto str_json = val.ToString();

  auto pretty_val = json.SerializePrettyToBytes();
  auto str_pretty = pretty_val.ToString();

  std::cout << "Json:\n" << str_pretty << std::endl;
  REQUIRE(answer == str_json);
}