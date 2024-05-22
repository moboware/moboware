project(boost_asio_ssl_server)

add_executable(asio_ssl_server
    server.cpp
)

target_link_libraries(asio_ssl_server PUBLIC
    moboware::socket
)

add_executable(asio_ssl_client
    client.cpp
)
target_link_libraries(asio_ssl_client PUBLIC
    moboware::socket
)