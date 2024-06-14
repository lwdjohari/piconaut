# The MIT License (MIT)
#
# FindH2OEvloop.cmake - Find libh2o-evloop and its dependencies
#
# This module defines
# H2O_EVLOOP_FOUND, whether h2o-evloop is found
# H2O_EVLOOP_INCLUDE_DIR, h2o-evloop headers location
# H2O_EVLOOP_LIBRARY, h2o-evloop library to link against
# OPENSSL_LIBRARIES, openssl library to link against 
# ZLIB_LIBRARIES, zlib library to link against
# H2O_EVLOOP_INCLUDE_DIRS, h2o-evloop headers location
# OPENSSL_INCLUDE_DIR, openssl headers location 
# ZLIB_INCLUDE_DIRS, zlib headers location
#
# Linggawasistha Djohari <linggawasistha.djohari@outlook.com>, 2024

option(H2O_USE_LIBUV OFF)
set(H2O_USE_LIBUV OFF)

# Locate the libh2o-evloop include directory
find_path(H2O_EVLOOP_INCLUDE_DIR
  NAMES h2o.h
  PATHS /usr/local/include /usr/include
)




# Locate the libh2o-evloop library
find_library(H2O_EVLOOP_LIBRARY
  NAMES h2o-evloop
  PATHS /usr/local/lib /usr/lib
)

# Find other required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUV REQUIRED libuv)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

# Print messages if any dependency is not found
if (NOT H2O_EVLOOP_INCLUDE_DIR)
  message(FATAL_ERROR "H2O evloop include directory not found.")
endif()

if (NOT H2O_EVLOOP_LIBRARY)
  message(FATAL_ERROR "H2O evloop library not found.")
endif()

# Handle the REQUIRED arguments and set H2O_EVLOOP_FOUND to TRUE if all components are found
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(H2O_EVLOOP REQUIRED_VARS H2O_EVLOOP_INCLUDE_DIR H2O_EVLOOP_LIBRARY)

if (H2O_EVLOOP_FOUND)
  set(H2O_EVLOOP_LIBRARIES ${H2O_EVLOOP_LIBRARY} ${LIBUV_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES})
  set(H2O_EVLOOP_INCLUDE_DIRS ${H2O_EVLOOP_INCLUDE_DIR} ${LIBUV_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS})
endif()

mark_as_advanced(H2O_EVLOOP_INCLUDE_DIR H2O_EVLOOP_LIBRARY)
