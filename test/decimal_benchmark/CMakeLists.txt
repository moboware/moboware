project(decimal_benchmark)

add_executable(${PROJECT_NAME}
    decimal_benchmark.cpp
)


target_link_libraries(${PROJECT_NAME} PRIVATE
                        benchmark::benchmark
                        moboware::common
    )
