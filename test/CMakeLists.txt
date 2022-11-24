find_package(GTest CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)

add_subdirectory(common_test)
add_subdirectory(modules_test)
add_subdirectory(web_socket_server_test_app)