project(binance)

add_library(${PROJECT_NAME}
    # files
    binance_stream_parser.cpp
    )

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

add_library(moboware::exchange::binance ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC
                         moboware::exchange
                         moboware::socket
                    )
