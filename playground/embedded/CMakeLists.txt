
project(embedded_python)

find_package(Python3 REQUIRED)
#find_package(Python3 COMPONENTS Development REQUIRED)
#target_link_libraries(main PRIVATE Python3::Python)

add_executable(${PROJECT_NAME}
    main.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/include/python3.10/)

target_link_libraries(${PROJECT_NAME} PUBLIC
          moboware::common
          python3.10
          Boost::python3.10
)