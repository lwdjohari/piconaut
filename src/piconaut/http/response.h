#pragma once
#include <h2o.h>

#include <string>
#include <stdexcept>
#include "piconaut/macro.h"

PICONAUT_INNER_NAMESPACE(http)

class Response {
 public:
  explicit Response(h2o_req_t* req);

  void set_status(int status_code) const;
  void set_header(const std::string& name, const std::string& value) const;
  void send(const std::string& body, int status_code = 200) const;
  void send_json(const std::string& json, int status_code = 200) const;

 private:
  h2o_req_t* req_;
};

PICONAUT_INNER_END_NAMESPACE