project(modules_test)

enable_testing()

add_executable(${PROJECT_NAME}  main.cpp 
                                order_book_test.cpp)

#target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

target_link_libraries(${PROJECT_NAME} PRIVATE 
                        GTest::gmock 
                        GTest::gtest 
                        moboware::modules
    )

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})