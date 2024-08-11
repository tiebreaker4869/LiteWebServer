cmake_minimum_required(VERSION 3.22.1)

project(LiteWebServer
        LANGUAGES C CXX
        DESCRIPTION "A Lightweight C++ Linuex Webserver."
)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)

include_directories(src)

# add lib mysql client dev
find_package(mysqlcppconn REQUIRED)
include_directories(${mysqlcppconn_INCLUDE_DIRS})

# add all .cpp to SOURCES(both sub and recursive sub dir)
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})