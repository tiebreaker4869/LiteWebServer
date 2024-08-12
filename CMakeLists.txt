cmake_minimum_required(VERSION 3.22.1)

project(LiteWebServer
    LANGUAGES C CXX
    DESCRIPTION "A Lightweight C++ Linux Web Server."
)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找 Google Test
find_package(GTest REQUIRED)

# 查找 MySQL 客户端库
include(FindPkgConfig)
pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)

# 添加源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp" ${SOURCES})

# exclude src/main.cpp from TEST_SOURCES
list(REMOVE_ITEM TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 添加测试
add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})

# 设置编译选项和包含目录
target_include_directories(${PROJECT_NAME} PRIVATE src ${GTest_INCLUDE_DIRS} ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME}_test PRIVATE src ${GTest_INCLUDE_DIRS} ${LIBMYSQLCLIENT_INCLUDE_DIRS})

# 设置编译选项
target_compile_options(${PROJECT_NAME} PRIVATE ${LIBMYSQLCLIENT_CFLAGS_OTHER})
target_compile_options(${PROJECT_NAME}_test PRIVATE ${LIBMYSQLCLIENT_CFLAGS_OTHER})

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE pthread ${LIBMYSQLCLIENT_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_test PRIVATE GTest::GTest GTest::Main pthread ${LIBMYSQLCLIENT_LIBRARIES})

# 启用测试
enable_testing()
add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
