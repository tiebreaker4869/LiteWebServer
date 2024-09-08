include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)

add_library(epoll STATIC ${CMAKE_CURRENT_SOURCE_DIR}/epoll.cpp)