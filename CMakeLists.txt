cmake_minimum_required(VERSION 3.22.1)

project(LiteWebServer
    LANGUAGES C CXX
    DESCRIPTION "A Lightweight C++ Linux Web Server."
)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(src)

# 查找 Google Test
find_package(GTest REQUIRED)

# 包含 Google Test 头文件
include_directories(${GTest_INCLUDE_DIRS})

# 查找 MySQL 客户端库
include(FindPkgConfig)
pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)

foreach(FLAG ${LIBMYSQLCLIENT_CFLAGS_OTHER})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
endforeach()

link_directories(${LIBMYSQLCLIENT_LIBRARY_DIRS})

# 添加源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接库
target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(${PROJECT_NAME} ${LIBMYSQLCLIENT_LIBRARIES})

# 添加测试
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})

# 链接测试库
target_link_libraries(${PROJECT_NAME}_test GTest::GTest GTest::Main pthread)

# 启用测试
enable_testing()
add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
