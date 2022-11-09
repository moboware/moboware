project(web_socket_server)

add_library(${PROJECT_NAME}
    web_socket_server.cpp
    web_socket_session.cpp
    
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(${PROJECT_NAME} PUBLIC "/home/mobo/code/sdk/websocketpp/")

add_library(moboware::web_socket_server ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    moboware::common
    jsoncpp_lib)
