include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)

add_library(http_conn STATIC ${CMAKE_CURRENT_SOURCE_DIR}/httpconn.cpp)
add_library(http_request STATIC ${CMAKE_CURRENT_SOURCE_DIR}/http_request.cpp)
add_library(http_response STATIC ${CMAKE_CURRENT_SOURCE_DIR}/http_response.cpp)

target_link_libraries(http_conn log http_request http_response)
target_link_libraries(http_request log sqlconnpool buffer timer)
target_link_libraries(http_response log sqlconnpool buffer timer)
