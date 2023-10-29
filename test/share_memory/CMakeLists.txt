project(shared_memory_test_app)

add_executable(${PROJECT_NAME}
        publisher.cpp
        consumer.cpp
        shared_memory.cpp
        main.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

target_link_libraries(${PROJECT_NAME}
    moboware::common
)