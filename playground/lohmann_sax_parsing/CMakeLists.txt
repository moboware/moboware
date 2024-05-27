project(lohmann_sax_parsing)

add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    moboware::common
    moboware::exchange::binance
    moboware::exchange

)