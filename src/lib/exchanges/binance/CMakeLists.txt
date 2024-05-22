project(binance)

add_library(${PROJECT_NAME}
    # files
    binance_price_feed.cpp
    binance_price_session_handler.cpp
    )

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

add_library(moboware::exchanges::binance ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC
                         moboware::common
                         moboware::socket
                    )
