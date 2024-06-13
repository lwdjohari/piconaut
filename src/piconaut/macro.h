#pragma once

// #ifndef H2O_USE_LIBUV
// #define H2O_USE_LIBUV 0
// #endif

#if __cplusplus >= 201703L
#define __PCN_CPP17 1
#else
#define __PCN_CPP14 1
#endif

#if __PCN_CPP17
#define __PCN_RETURN_MOVE(arg) arg
#else
#define __PCN_RETURN_MOVE(arg) std::move(arg)
#endif

#if __PCN_CPP17
#include <string_view>
#endif

#if __PCN_CPP17
#define __PCN_STRING_COMPAT std::string_view
#else
#define __PCN_STRING_COMPAT std::string
#endif

#if __PCN_CPP17
#define __PCN_STRING_COMPAT_REF std::string_view
#else
#define __PCN_STRING_COMPAT_REF std::string &
#endif

#if __PCN_CPP17
#define __PCN_CONSTEXPR constexpr
#else
#define __PCN_CONSTEXPR
#endif

#define PICONAUT_NAMESPACE namespace piconaut
#define PICONAUT_BEGIN_ROOT_NAMESPACE \
    PICONAUT_NAMESPACE                \
    {
#define PICONAUT_INNER_NAMESPACE(arg) \
    PICONAUT_BEGIN_ROOT_NAMESPACE     \
    namespace arg                   \
    {
#define PICONAUT_END_NAMESPACE }

#define PICONAUT_INNER_END_NAMESPACE }}

