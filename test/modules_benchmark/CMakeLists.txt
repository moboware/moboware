project(modules_benchmark)

add_executable(${PROJECT_NAME}
    modules_benchmark.cpp
)


target_link_libraries(${PROJECT_NAME} PRIVATE
                        benchmark::benchmark
                        benchmark::benchmark_main
                        moboware::modules
    )
