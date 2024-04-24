project(modules_benchmark)

add_executable(${PROJECT_NAME}
    main.cpp
    order_event_processor_benchmark.cpp
    matching_engine_benchmark.cpp
)


target_link_libraries(${PROJECT_NAME} PRIVATE
                        benchmark::benchmark
                        benchmark::benchmark_main
                        moboware::modules
    )
