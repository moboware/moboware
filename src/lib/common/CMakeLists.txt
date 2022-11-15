project(${PROJECT_NAME})

add_library(${PROJECT_NAME}
    service.cpp
    application_base.cpp
    channel_base.cpp
    timer.cpp
    tcp_server.cpp
    tcp_client.cpp
    session.cpp
    log.cpp
    log_stream.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

add_library(moboware::common ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC 
                        ${jsoncpp_LIBRARIES}
                        Threads::Threads)