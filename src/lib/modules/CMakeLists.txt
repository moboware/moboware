project(modules)

add_library(${PROJECT_NAME}
    tcp_client_module.cpp
    log_module.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

add_library(moboware::modules ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    moboware::common
    jsoncpp_lib)
