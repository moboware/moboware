
project(new_logger)


add_executable(${PROJECT_NAME}
    main.cpp
)


target_link_libraries(${PROJECT_NAME} PUBLIC
    moboware::common
)