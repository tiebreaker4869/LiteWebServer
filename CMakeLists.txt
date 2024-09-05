cmake_minimum_required(VERSION 3.20)

project(LiteWebserver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_DIR src)
set(HEADER_DIR include)

add_executable(LiteWebserver ${SOURCE_DIR}/main.cpp)

target_include_directories(LiteWebserver PRIVATE ${HEADER_DIR})
