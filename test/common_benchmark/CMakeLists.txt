project(common_benchmark)

add_executable(${PROJECT_NAME}
    log_stream_benchmark.cpp
)


target_link_libraries(${PROJECT_NAME} PRIVATE
                        benchmark::benchmark
                        moboware::common
    )
