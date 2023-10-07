project(object_copy_benchmark)

add_executable(${PROJECT_NAME}
    object_copy_benchmark.cpp
)


target_link_libraries(${PROJECT_NAME} PRIVATE
                        benchmark::benchmark
                        moboware::common
    )
