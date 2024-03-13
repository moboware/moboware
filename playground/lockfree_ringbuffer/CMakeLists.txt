
project(lockfree_ringbuffer)


add_executable(${PROJECT_NAME}
    main.cpp
)


target_link_libraries(${PROJECT_NAME} PUBLIC
          moboware::common
)