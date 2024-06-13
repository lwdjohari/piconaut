
#include "piconaut/http/response.h"

// cppcheck-suppress unknownMacro
PICONAUT_INNER_NAMESPACE(http)
Response::Response(h2o_req_t* req) : req_(req) {
  if (!req_) {
    throw std::invalid_argument("Response object cannot be null");
  }
}

void Response::set_status(int status_code) const {
  req_->res.status = status_code;
}

void Response::set_header(const std::string& name,
                          const std::string& value) const {
  h2o_add_header_by_str(&req_->pool, &req_->res.headers, name.c_str(),
                        name.size(), 1, nullptr, value.c_str(), value.size());
}

void Response::send(const std::string& body, int status_code) const {
  set_status(status_code);
  req_->res.reason = "OK";

  h2o_iovec_t body_iovec = h2o_iovec_init(body.c_str(), body.size());
  h2o_send(req_, &body_iovec, 1, H2O_SEND_STATE_FINAL);
}

void Response::send_json(const std::string& json, int status_code) const {
  set_status(status_code);
  set_header("Content-Type", "application/json");
  req_->res.reason = "OK";

  h2o_iovec_t json_iovec = h2o_iovec_init(json.c_str(), json.size());
  h2o_send(req_, &json_iovec, 1, H2O_SEND_STATE_FINAL);
}

PICONAUT_INNER_END_NAMESPACE