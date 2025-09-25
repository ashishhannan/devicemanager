#include "server.hpp"
#include "session.hpp"
#include <iostream>

Server::Server(boost::asio::io_context& ioc, uint16_t port, DB& db, DeviceRegistry& registry)
    : ioc_(ioc),
      acceptor_(ioc_, tcp::endpoint(tcp::v4(), port)),
      db_(db),
      registry_(registry)
{
    do_accept();
}

void Server::do_accept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                // Create session and pass DB + registry references
                std::make_shared<Session>(std::move(socket), db_, registry_)->start();
            } else {
                std::cerr << "Accept error: " << ec.message() << "\n";
            }
            // continue accepting
            do_accept();
        });
}

