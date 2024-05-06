project(socket)

add_library(${PROJECT_NAME}
    socket_session_base.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

add_library(moboware::socket ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    moboware::common
)
