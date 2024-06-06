project(market_data_feed)

add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    moboware::exchange::binance
    moboware::exchange::bitstamp
)