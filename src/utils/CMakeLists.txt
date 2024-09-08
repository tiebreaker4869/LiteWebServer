include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)

add_library(buffer STATIC ${CMAKE_CURRENT_SOURCE_DIR}/buffer.cpp)
add_library(timer STATIC ${CMAKE_CURRENT_SOURCE_DIR}/timer.cpp)
add_library(log STATIC ${CMAKE_CURRENT_SOURCE_DIR}/log.cpp)

target_link_libraries(log buffer pthread)


