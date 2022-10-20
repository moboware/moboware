cmake_minimum_required(VERSION 3.22)
project(moboware LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) # websocketpp only supports c++ 17
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release")

find_package(jsoncpp REQUIRED)

include_directories(${jsoncpp_INCLUDE_DIRS})

add_subdirectory(src)
add_subdirectory(test)