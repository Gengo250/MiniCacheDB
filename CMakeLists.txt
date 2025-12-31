cmake_minimum_required(VERSION 3.20)
project(MiniCacheDB LANGUAGES CXX)

option(MINICACHEDB_SANITIZE "Enable Address/UB sanitizers" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(minicachedb_core
  src/core/thread_pool.cpp
  src/core/kv_store.cpp
  src/core/protocol.cpp
  src/core/aof.cpp
  src/net/tcp_server.cpp
)

target_include_directories(minicachedb_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (MSVC)
  target_compile_options(minicachedb_core PRIVATE /W4)
else()
  target_compile_options(minicachedb_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if (MINICACHEDB_SANITIZE AND NOT MSVC)
  target_compile_options(minicachedb_core PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(minicachedb_core PRIVATE -fsanitize=address,undefined)
endif()

add_executable(minicachedb src/main.cpp)
target_link_libraries(minicachedb PRIVATE minicachedb_core)

enable_testing()

add_executable(test_kv_store tests/test_kv_store.cpp)
target_link_libraries(test_kv_store PRIVATE minicachedb_core)
add_test(NAME kv_store COMMAND test_kv_store)

add_executable(test_protocol tests/test_protocol.cpp)
target_link_libraries(test_protocol PRIVATE minicachedb_core)
add_test(NAME protocol COMMAND test_protocol)
