project(smart_search_benchmark)


add_executable(${PROJECT_NAME}
    smart_search_benchmark.cpp
)


target_link_libraries(${PROJECT_NAME} PRIVATE
                        benchmark::benchmark
                        moboware::common
    )

