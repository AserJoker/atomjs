cmake_minimum_required(VERSION 3.12)
project(atomjs)
set(ATOMJS_RUNTIME_NAME atom)
set(ATOMJS_RUNTIME_NAME_TEST atomTest)
set(ATOMJS_COMPILER_NAME atomc)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
include_directories(./src)
include_directories(./quickjs/include)
add_executable(${ATOMJS_RUNTIME_NAME} ${PROJECT_SOURCE_DIR}/src/main.cpp)
add_subdirectory(quickjs)

target_link_libraries(${ATOMJS_RUNTIME_NAME} PRIVATE quickjs pthread)

find_package(cJSON CONFIG REQUIRED)
target_link_libraries(${ATOMJS_RUNTIME_NAME} PRIVATE cjson)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${ATOMJS_RUNTIME_NAME} PRIVATE fmt::fmt)

add_executable(${ATOMJS_RUNTIME_NAME_TEST} ${PROJECT_SOURCE_DIR}/test/pathTest.cpp)

target_link_libraries(${ATOMJS_RUNTIME_NAME_TEST} PRIVATE quickjs pthread)

find_package(cJSON CONFIG REQUIRED)
target_link_libraries(${ATOMJS_RUNTIME_NAME_TEST} PRIVATE cjson)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${ATOMJS_RUNTIME_NAME_TEST} PRIVATE fmt::fmt)
find_package(GTest CONFIG REQUIRED)
target_link_libraries(${ATOMJS_RUNTIME_NAME_TEST} PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

enable_testing()
add_test(path ${PROJECT_SOURCE_DIR}/${ATOMJS_RUNTIME_NAME_TEST})