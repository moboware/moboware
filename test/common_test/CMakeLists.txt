project(common_test)

enable_testing()

add_executable(${PROJECT_NAME}
    ring_buffer_test.cpp
    main.cpp
)


target_link_libraries(${PROJECT_NAME} PRIVATE
                        GTest::gmock
                        GTest::gtest
                        moboware::common
    )

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})