#pragma once
#include <string>
#include <memory>

#include "piconaut/macro.h"
#include "nvm/macro.h"
// cppcheck-suppress unknownMacro
PICONAUT_INNER_NAMESPACE(http)

class Config;
class HandlerBase;
class MiddlewareManager;
class Request;
class Response;
using MiddlewareManagerPtr = std::shared_ptr<MiddlewareManager>;
using HandlerBasePtr = std::shared_ptr<HandlerBase>;
using ConfigPtr = std::shared_ptr<Config>;

enum class HttpVersionMode {
    HTTP1_1,
    HTTP2,
    HTTP2_AUTO_FALLBACK
};

enum class CompressionType {
    NONE,
    GZIP,
    BROTLI
};


PICONAUT_INNER_END_NAMESPACE