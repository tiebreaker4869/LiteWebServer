
add_subdirectory(utils)
add_subdirectory(sql)
add_subdirectory(server)
add_subdirectory(net)
add_subdirectory(http)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_executable(main main.cpp)

target_link_libraries(main webserver)