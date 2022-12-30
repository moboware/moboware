project(modules_test)

enable_testing()

add_executable(${PROJECT_NAME}  main.cpp
                                order_book_test.cpp
                                order_eent_procesor_test.cpp)


target_link_libraries(${PROJECT_NAME} PRIVATE
                        GTest::gmock
                        GTest::gtest
                        moboware::modules
    )

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})