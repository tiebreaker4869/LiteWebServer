include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)

add_library(webserver STATIC ${CMAKE_CURRENT_SOURCE_DIR}/webserver.cpp)

target_link_libraries(webserver log buffer sqlconnpool http_request http_response 
    http_conn epoll pthread)
