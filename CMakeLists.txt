cmake_minimum_required(VERSION 3.10)
project(piconaut CXX)

message( STATUS "Piconout REST FRAMEWORK v1")
message( STATUS "-----------------------------")
# set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
# Main server development options

#  feature
# Network
option(PCN_FEATURE_ENVOY "Integrate Envoy Dataplane Library" OFF)

# Print the Piconout FEATURES
message(STATUS "Piconout FEATURES.")
message(STATUS "Core: ON")

message(STATUS "NvLog: ON")

option(H2O_USE_LIBUV OFF)
set(H2O_USE_LIBUV OFF)

message(STATUS "")

message(STATUS "Piconout LIB DIR:${CMAKE_CURRENT_SOURCE_DIR}")


include(ProjectCXX)
set(ISROOT FALSE)
NV_DETECT_IS_ROOT_PROJECT(ISROOT)
message(STATUS "Piconout Root Project: ${ISROOT} ${PCN_CXX_VERSION}")


option(PCN_BUILD_DOC "Build documentation" OFF)
option(PCN_SANITIZE_ADDRESS "SANITIZE ADDRESS" ON)
option(PCN_USE_CATCH ON)
option(PCN_USE_TEST ON)
option(PCN_BUILD_EXAMPLE ON)

if(ISROOT)
  if(NOT PCN_CXX_VERSION)
    set(PCN_CXX_VERSION 17)
  endif()
    option(PCN_CXX_STANDARD_REQUIRED "CXX Required" ON)
    option(PCN_CXX_EXTENSIONS "CXX Extensions" ON)
  NV_PROPOGATE_CXX_OPTIONS(${PCN_CXX_VERSION} ${PCN_CXX_STANDARD_REQUIRED} ${PCN_CXX_EXTENSIONS})
  set(PCN_USE_CATCH ON)
  set(PCN_USE_TEST ON)
  set(PCN_BUILD_EXAMPLE ON)
  set(CXX_FEATURE "")
else()
    set(NVSERV_CXX_VERSION ${CMAKE_CXX_STANDARD})
endif()


NV_GET_CXX_STD_FEATURE(${PCN_CXX_VERSION} CXX_FEATURE)
message(STATUS "CXX Feature: ${CXX_FEATURE}")

# Add ASAN
if(PCN_SANITIZE_ADDRESS)
    set(SANITIZE_ADDRESS ON)
endif(PCN_SANITIZE_ADDRESS)

#  make ourself sanitize and sane well.
if(NOT Sanitizers_FOUND)
 find_package(Sanitizers)
endif()

# Add Doxygen
if(PCN_BUILD_DOC)
    if(NOT DOXYGEN_FOUND)
        find_package(Doxygen)
    endif()

    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_SOURCE_DIR}/generated/docs-cached/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

         # note the option ALL which allows to build the docs together with the application
        add_custom_target( doc_piconout ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/generated/docs-cached/
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endif(PCN_BUILD_DOC)

# Add Unit Test Lib Catch2
if(PCN_USE_CATCH)
 message(STATUS  "CATCH2::ADD_LIB.")
 find_package(Catch2 3 REQUIRED)
endif()

find_package(Threads REQUIRED)

# Include custom FindH2O module
# find_package(Wslay REQUIRED)
# if(NOT WSLAY_FOUND)
#     message(FATAL_ERROR "wslay library not found")
# endif()

message(STATUS  "LibH2O: ON")
find_package(H2O_EVLOOP REQUIRED)

if (NOT nvcore_FOUND)
    message(STATUS  "NvCore: ON")
    add_subdirectory(deps/nvcore build-nvcore)
endif()

add_subdirectory(deps/nvlog build-nvlog)


#rapidjson
message(STATUS  "Rapidjson: ON")
set(RAPIDJSON_BUILD_CXX11 ON)
set(RAPIDJSON_BUILD_DOC  OFF)
set(RAPIDJSON_BUILD_EXAMPLES  OFF)
set(RAPIDJSON_BUILD_TESTS  OFF)
set(RAPIDJSON_BUILD_THIRDPARTY_GTEST OFF)
add_subdirectory(deps/rapidjson build-rapidjson)

# piconout-framework
# combine all needed linking

#Main headers and sources
file(GLOB_RECURSE SOURCES_PICONAUT CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.s
)


# nvserv::core
add_library(${PROJECT_NAME} STATIC ${SOURCES_PICONAUT} )
target_link_libraries(${PROJECT_NAME}  
    PUBLIC
        nvcore
    PRIVATE
        ${H2O_EVLOOP_LIBRARIES}
        #${H2O_LIBRARIES}
    )
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(${PROJECT_NAME} PUBLIC "H2O_USE_LIBUV=0")

target_compile_features(${PROJECT_NAME} PUBLIC ${CXX_FEATURE})
target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/rapidjson/include
    # ${H2O_INCLUDE_DIRS}
    ${H2O_EVLOOP_INCLUDE_DIRS}

)

message(STATUS "H2O-EVLOOP:\n${H2O_EVLOOP_LIBRARIES}\n${H2O_EVLOOP_INCLUDE_DIRS}")

add_library(${PROJECT_NAME}::piconaut ALIAS ${PROJECT_NAME} )

add_executable(${PROJECT_NAME}-runner main.cc)
target_link_libraries(${PROJECT_NAME}-runner  
    PUBLIC
        ${PROJECT_NAME}::piconaut
    )
set_target_properties(${PROJECT_NAME}-runner PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(${PROJECT_NAME}-runner PUBLIC ${CXX_FEATURE})

# target_include_directories(${PROJECT_NAME}-runner
#     PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/
# )

if(PCN_USE_TEST)
    message(STATUS "Piconaut Test: ON")
    add_subdirectory(tests/ build-piconaut-test)
endif()       

message(STATUS "Piconout LIB Configuration Done!\n")
set(piconout_FOUND  ON)

if(PCN_BUILD_EXAMPLE)
    
endif()

message(STATUS "")