cmake_minimum_required(VERSION 3.11)

if(APPLE)
    set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
elseif(UNIX)
    set(CMAKE_C_COMPILER "/usr/bin/clang")
endif()

project(dnsRelay C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED 11)

add_compile_options(-O2 -Wall)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(src)