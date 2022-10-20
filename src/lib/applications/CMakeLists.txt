project(applications)

add_library(${PROJECT_NAME}
    application.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

add_library(moboware::applications ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    moboware::channels
    jsoncpp_lib)
