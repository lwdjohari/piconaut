#pragma once
#include <h2o.h>

#include <string>
#include <stdexcept>
#include "piconaut/macro.h"
#include "piconaut/formats/json/value.h"

PICONAUT_INNER_NAMESPACE(http)

class Response {
 public:
  explicit Response(h2o_req_t* req);

  void Status(int status_code) const;
  void AddHeader(const std::string& name, const std::string& value) const;
  void Send(const std::string& body, int status_code = 200) const;
  void SendJson(const formats::json::Value& json, int status_code = 200) const;

 private:
  h2o_req_t* req_;
};

PICONAUT_INNER_END_NAMESPACE