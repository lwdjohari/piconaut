#pragma once

#include "piconaut/macro.h"
#include "piconaut/http/declare.h"
#include "piconaut/http/impl/h2o_impl.h"
// #include "piconout/http/config.h"

PICONAUT_INNER_NAMESPACE(http)

class HttpServer
{
private:
public:
    HttpServer();
    ~HttpServer();
};

PICONAUT_INNER_END_NAMESPACE