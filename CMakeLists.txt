cmake_minimum_required(VERSION 3.22)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Found ccache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
else()
  message("Install ccache with:sudo apt install ccache")
endif()

project(moboware LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release")

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE dynamic) # This changed from static to dynamic
set(VCPKG_CMAKE_SYSTEM_NAME Linux)


find_package(OpenSSL REQUIRED)
find_package(Threads)
find_package(Boost)

include_directories(${Boost_INCLUDE_DIRS})
find_package(jsoncpp CONFIG REQUIRED)

add_subdirectory(src)
add_subdirectory(test)