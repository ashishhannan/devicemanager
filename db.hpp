#pragma once
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <cstdint>

class DB {
public:
    DB(const std::string& conninfo);
    void store_raw_key(const std::string& device_imei, uint8_t command, uint8_t key, const std::vector<uint8_t>& value, uint16_t seq);
    // convenience methods for decoded data
    void store_gps(const std::string& device_imei, uint32_t timestamp, double lat, double lon, uint16_t seq);
private:
    pqxx::connection conn_;
};

