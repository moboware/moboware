project(web_socket_server_test_app)

add_executable(${PROJECT_NAME} main.cpp)
    
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

target_link_libraries(${PROJECT_NAME}
    moboware::channels
    moboware::web_socket_server)