project(channels)

add_library(${PROJECT_NAME}
web_socket_channel.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

add_library(moboware::channels ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    moboware::common
    moboware::modules
    moboware::web_socket
    )
