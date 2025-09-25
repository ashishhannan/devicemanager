#include "session.hpp"
#include <iostream>

Session::Session(tcp::socket socket, DB& db)
    : socket_(std::move(socket)), db_(db) {}

void Session::start() {
    do_read_header();
}

void Session::do_read_header() {
    auto self = shared_from_this();
    read_buf_.resize(1 + 1 + 2 + 2 + 2); // header + properties + len(2) + crc(2) + seq(2)
    boost::asio::async_read(socket_, boost::asio::buffer(read_buf_),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (ec) {
                // connection closed
                return;
            }
            // validate header
            if (read_buf_[0] != 0xAB) {
                // drop and attempt to read next byte (resync strategy could be improved)
                do_read_header();
                return;
            }
            uint16_t len = uint16_t(read_buf_[2]) | (uint16_t(read_buf_[3]) << 8);
            // now read the rest (message body of len bytes)
            // note: we've already read checksum and seq id bytes above
            size_t remaining = len + 1 /*command at least*/;
            // but frame is: header(1)+prop(1)+len(2)+crc(2)+seq(2)+body(len)
            size_t total_frame_size = 1+1+2+2+2 + len;
            // read remainder
            read_buf_.resize(total_frame_size);
            boost::asio::async_read(socket_, boost::asio::buffer(&read_buf_[1+1+2+2+2], len),
                [this, self, total_frame_size](boost::system::error_code ec2, std::size_t l2) {
                    if (ec2) return;
                    // at this point read_buf_ contains the full frame
                    process_frame(read_buf_);
                    // continue reading next frame
                    do_read_header();
                });
        });
}

void Session::process_frame(const std::vector<uint8_t>& frame) {
    // -----------------------------
    // RECEIVED FROM DEVICE: frame
    // -----------------------------
    auto opt = ProtocolParser::parse_frame(frame);
    if (!opt) {
        std::cerr << "Frame parse / CRC failed. Ignoring\n";
        return;
    }
    ParsedMessage pm = *opt;

    // find Device IMEI key (0x01) in keys
    std::string device_imei = "unknown";
    for (const auto& kv : pm.keys) {
        if (kv.key_id == 0x01) {
            // IMEI is 15 bytes ASCII encoded as hex-bytes (doc)
            device_imei = std::string(kv.value.begin(), kv.value.end());
            break;
        }
    }

    // store raw keys into DB
    for (const auto& kv : pm.keys) {
        db_.store_raw_key(device_imei, pm.command, kv.key_id, kv.value, pm.seq_id);

        // quick decode for GPS (0x20) and store into gps table
        if (kv.key_id == 0x20 && kv.value.size() >= 22 - 1) { // documented length 0x16 -> subtract 1 for key byte already removed.
            // bytes 0-3 lat (signed, ten-millionth of degree)
            int32_t lat = (int32_t(kv.value[0]) | (int32_t(kv.value[1])<<8) | (int32_t(kv.value[2])<<16) | (int32_t(kv.value[3])<<24));
            int32_t lon = (int32_t(kv.value[4]) | (int32_t(kv.value[5])<<8) | (int32_t(kv.value[6])<<16) | (int32_t(kv.value[7])<<24));
            // convert to decimal degrees
            double dlat = lat / 1e7;
            double dlon = lon / 1e7;
            // timestamp typically in another key; for demonstration we use seq_id as placeholder
            db_.store_gps(device_imei, 0, dlat, dlon, pm.seq_id);
        }
    }

    // If ACK requested in properties (bit4)
    bool ack_requested = (pm.properties & (1<<4)) != 0;
    if (ack_requested) {
        // Build ACK and send
        auto ack = ProtocolParser::build_ack(pm.seq_id, pm.command);
        auto self = shared_from_this();
        boost::asio::async_write(socket_, boost::asio::buffer(ack),
            [this, self](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Failed sending ACK: " << ec.message() << "\n";
                } else {
                    // -----------------------------
                    // SENT TO DEVICE: ACK frame
                    // -----------------------------
                }
            });
    }
}

