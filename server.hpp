#pragma once
#include <boost/asio.hpp>
#include "db.hpp"
#include "device_registry.hpp"
using boost::asio::ip::tcp;

class Server {
public:
    Server(boost::asio::io_context& ioc, uint16_t port, DB& db, DeviceRegistry& registry);
    void start_accept();

private:
    void do_accept();

    boost::asio::io_context& ioc_;
    tcp::acceptor acceptor_;
    DB& db_;
    DeviceRegistry& registry_;
};

