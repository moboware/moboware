project(common_benchmark)

add_executable(${PROJECT_NAME}
    main.cpp
    # log_stream_benchmark.cpp
    fast_map_benchmark.cpp
)


target_link_libraries(${PROJECT_NAME} PRIVATE
                        benchmark::benchmark
                        moboware::common
    )
