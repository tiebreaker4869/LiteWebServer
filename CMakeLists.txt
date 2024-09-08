cmake_minimum_required(VERSION 3.20)

project(LiteWebserver)

set(CXX_FLAGS
 -O2
 -Wall
 -Wextra
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native
 -std=c++17
 -rdynamic
)

add_subdirectory(src)
