include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)

add_library(sqlconnpool STATIC ${CMAKE_CURRENT_SOURCE_DIR}/sqlconnpool.cpp)

target_link_libraries(sqlconnpool mysqlclient)