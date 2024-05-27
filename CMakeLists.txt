cmake_minimum_required(VERSION 3.22)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Found ccache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
else()
  message("Install ccache with:sudo apt install ccache")
endif()

project(moboware LANGUAGES CXX)

# options
option(PERFORMANCE_BUILD_FLAGS "Enable perf tool performance compiler flags " OFF)

# sets
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release")

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE dynamic) # This changed from static to dynamic
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

set(CMAKE_CXX_FLAGS "-DNDEBUG")

# set release build flags with or without performance profiling
set(RELEASE_FLAGS "-Ofast -DNDEBUG")
if(PERFORMANCE_BUILD_FLAGS)
  set(RELEASE_FLAGS "${RELEASE_FLAGS} -fno-omit-frame-pointer -pg")
  message("Build with perf tool flags:'${RELEASE_FLAGS}'")
endif()

set(CMAKE_CXX_FLAGS_RELEASE ${RELEASE_FLAGS}) # debug info in release mode

# find packges
# openssl package
find_package(OpenSSL REQUIRED)

# nlohmann json parser
# uasage: target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)
find_package(nlohmann_json CONFIG REQUIRED)
set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)

# boost package
set(Boost_USE_STATIC_LIBS On)
find_package(
  Boost
  COMPONENTS thread
             coroutine
             context
             date_time
             filesystem
             program_options
             system
  REQUIRED)

find_package(fmt REQUIRED)


include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(playground)