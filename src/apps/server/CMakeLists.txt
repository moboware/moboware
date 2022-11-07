project(web_socket_server_app)

add_executable(${PROJECT_NAME}
  web_socket_server.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

target_link_libraries(${PROJECT_NAME}
  moboware::applications
  jsoncpp_lib)
