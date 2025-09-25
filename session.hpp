#pragma once
#include <boost/asio.hpp>
#include <memory>
#include "protocol_parser.hpp"
#include "db.hpp"
using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, DB& db);
    void start();

private:
    void do_read_header();
    void do_read_body(size_t body_len);
    void process_frame(const std::vector<uint8_t>& frame);

    tcp::socket socket_;
    DB& db_;
    std::vector<uint8_t> read_buf_;
};

