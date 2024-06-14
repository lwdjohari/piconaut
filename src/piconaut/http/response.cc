
#include "piconaut/http/response.h"

// cppcheck-suppress unknownMacro
PICONAUT_INNER_NAMESPACE(http)
Response::Response(h2o_req_t* req) : req_(req) {
  if (!req_) {
    throw std::invalid_argument("Response object cannot be null");
  }
}

void Response::Status(int status_code) const {
  req_->res.status = status_code;
}

void Response::AddHeader(const std::string& name,
                         const std::string& value) const {
  h2o_add_header_by_str(&req_->pool, &req_->res.headers, name.c_str(),
                        name.size(), 1, nullptr, value.c_str(), value.size());
}

void Response::Send(const std::string& body, int status_code) const {
  try {
    Status(status_code);
    req_->res.reason = "OK";
    h2o_send_inline(req_, body.c_str(), body.size());
  } catch (const std::exception& e) {
    Status(H2O_STATUS_ERROR_503);
    h2o_send_error_503(req_, "internal server error", e.what(), 0);
  }
}

void Response::SendJson(const formats::json::Value& json, int status_code) const {
  try {
    Status(status_code);
    h2o_add_header(&req_->pool, &req_->res.headers, H2O_TOKEN_CONTENT_TYPE, NULL, H2O_STRLIT("application/json"));
    req_->res.reason = "OK";

    // Serialize the JSON to a byte array
    std::vector<uint8_t> byte_array = json.SerializeToBytes();

    // Prepare the buffer
    h2o_iovec_t body;
    body.base = reinterpret_cast<char*>(byte_array.data());
    body.len = byte_array.size();

    // Send the response
    h2o_send_inline(req_, body.base, body.len);
  } catch (const std::exception& e) {
    Status(H2O_STATUS_ERROR_503);
    h2o_send_error_503(req_, "internal server error", e.what(), 0);
  }
}

PICONAUT_INNER_END_NAMESPACE