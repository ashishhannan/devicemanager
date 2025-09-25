#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <optional>

struct KeyValue {
    uint8_t key_id;
    std::vector<uint8_t> value;
};

struct ParsedMessage {
    uint8_t properties;
    uint16_t length;
    uint16_t checksum;
    uint16_t seq_id;
    uint8_t command;
    std::vector<KeyValue> keys;
};

class ProtocolParser {
public:
    // parse a full frame (body already read as bytes input)
    // returns std::nullopt if CRC or framing invalid
    static std::optional<ParsedMessage> parse_frame(const std::vector<uint8_t>& frame);

    // Build a standard ACK frame (as per doc example)
    static std::vector<uint8_t> build_ack(uint16_t seq_id, uint8_t orig_command);

    // CRC16 calculation (uses algorithm/table from protocol doc).
    static uint16_t crc16_compute(const uint8_t* data, uint32_t len, uint16_t init = 0x0000);
};

