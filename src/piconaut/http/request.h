#pragma once

#include "piconaut/macro.h"
#include <string>
#include <unordered_map>
#include <h2o.h>
#include <stdexcept>

PICONAUT_INNER_NAMESPACE(http)


class Request {
 public:
    explicit Request(h2o_req_t* req);

    std::string get_path() const;
    std::string get_method() const;
    std::unordered_map<std::string, std::string> get_headers() const;
    std::string get_header(const std::string& name) const;
    std::string get_body() const;
    std::string get_query_param(const std::string& name) const;

 private:
    h2o_req_t* req_;
};




PICONAUT_INNER_END_NAMESPACE