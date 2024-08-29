cmake_minimum_required(VERSION 3.13)

include(PreLoad.cmake)

set(PROJECT_NAME crypto)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER clang++)

# for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# openssl
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
  MESSAGE(STATUS "current platform: Linux ")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
  MESSAGE(STATUS "current platform: Windows")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  MESSAGE(STATUS "current platform: FreeBSD")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
  MESSAGE(STATUS "current platform: MacOS")
  set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl" )
  set(OPENSSL_LIBRARIES "/usr/local/opt/openssl/lib" )
  set(OPENSSL_INCLUDE_DIR "/usr/local/opt/openssl/include" )
ELSE ()
  MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

set(OPENSSL_USE_STATIC_LIBS True)
find_package(OpenSSL 3 REQUIRED)
if ( OPENSSL_FOUND )
  message(STATUS "OpenSSL Found: ${OPENSSL_VERSION}")
  message(STATUS "OpenSSL Include: ${OPENSSL_INCLUDE_DIR}")
  message(STATUS "OpenSSL Libraries: ${OPENSSL_LIBRARIES}")
endif()
include_directories(${OPENSSL_INCLUDE_DIR})

# Boost
set(Boost_USE_STATIC_LIBS       OFF)
set(Boost_USE_DEBUG_LIBS        OFF)
set(Boost_USE_RELEASE_LIBS      ON)
set(Boost_USE_MULTITHREADED     ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.81 REQUIRED COMPONENTS system context thread filesystem json)
include_directories(${Boost_INCLUDE_DIR})

# quill - lib for logging
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(quill_DIR /opt/vcpkg/packages/quill_x64-linux/share/quill/)
  set(fmt_DIR /opt/vcpkg/packages/fmt_x64-linux/share/fmt/)
endif()
find_package(quill CONFIG REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_sources(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_SOURCE_DIR}/models/common.hpp
  ${CMAKE_SOURCE_DIR}/models/context.hpp
  ${CMAKE_SOURCE_DIR}/streams/binance.hpp
  ${CMAKE_SOURCE_DIR}/streams/mexc.hpp
  ${CMAKE_SOURCE_DIR}/streams/gateio.hpp
  ${CMAKE_SOURCE_DIR}/streams/base_stream.hpp
  ${CMAKE_SOURCE_DIR}/utils/logger.hpp
  ${CMAKE_SOURCE_DIR}/utils/scanner.hpp
  PRIVATE
  ${CMAKE_SOURCE_DIR}/models/context.cpp
  ${CMAKE_SOURCE_DIR}/streams/binance.cpp
  ${CMAKE_SOURCE_DIR}/streams/mexc.cpp
  ${CMAKE_SOURCE_DIR}/streams/gateio.cpp
  ${CMAKE_SOURCE_DIR}/streams/base_stream.cpp
  ${CMAKE_SOURCE_DIR}/utils/logger.cpp
  ${CMAKE_SOURCE_DIR}/utils/scanner.cpp
)
target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_SOURCE_DIR}/models
  ${CMAKE_SOURCE_DIR}/streams
  ${CMAKE_SOURCE_DIR}/utils
)

message("OPENSSL_LIBRARIES=${OPENSSL_LIBRARIES}")
message("Boost_LIBRARIES=${Boost_LIBRARIES}")
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE quill::quill)

# всякий мусор
message("CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
message("CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")
set(BUILD_TRASH ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${BUILD_TRASH} CACHE PATH "Build directory" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_TRASH})
set(CMAKE_BUILD_FILES_DIRECTORY ${BUILD_TRASH})
set(CMAKE_BUILD_DIRECTORY ${BUILD_TRASH})
SET(CMAKE_CACHEFILE_DIR ${BUILD_TRASH})
message("CMAKE_BUILD_DIRECTORY=${CMAKE_BUILD_DIRECTORY}")
