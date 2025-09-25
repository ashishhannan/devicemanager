#include "db.hpp"
#include <iostream>
#include <sstream>

DB::DB(const std::string& conninfo) : conn_(conninfo) {
    // create table if not exists (idempotent)
    pqxx::work w(conn_);
    w.exec(R"(
        CREATE TABLE IF NOT EXISTS device_messages (
            id SERIAL PRIMARY KEY,
            device_imei TEXT,
            seq_id INT,
            command INT,
            key_id INT,
            raw_value BYTEA,
            created_at TIMESTAMP DEFAULT NOW()
        );
        CREATE TABLE IF NOT EXISTS gps (
            id SERIAL PRIMARY KEY,
            device_imei TEXT,
            seq_id INT,
            ts BIGINT,
            lat DOUBLE PRECISION,
            lon DOUBLE PRECISION,
            created_at TIMESTAMP DEFAULT NOW()
        );
    )");
    w.commit();
}

void DB::store_raw_key(const std::string& device_imei, uint8_t command, uint8_t key, const std::vector<uint8_t>& value, uint16_t seq) {
    try {
        pqxx::work w(conn_);
        w.exec_params("INSERT INTO device_messages (device_imei, seq_id, command, key_id, raw_value) VALUES ($1,$2,$3,$4,$5)",
                      device_imei, (int)seq, (int)command, (int)key, pqxx::binarystring(value.data(), value.size()));
        w.commit();
    } catch (const std::exception &e) {
        std::cerr << "DB raw insert error: " << e.what() << "\n";
    }
}

void DB::store_gps(const std::string& device_imei, uint32_t timestamp, double lat, double lon, uint16_t seq) {
    try {
        pqxx::work w(conn_);
        w.exec_params("INSERT INTO gps (device_imei, seq_id, ts, lat, lon) VALUES ($1,$2,$3,$4,$5)",
                      device_imei, (int)seq, (int64_t)timestamp, lat, lon);
        w.commit();
    } catch (const std::exception &e) {
        std::cerr << "DB GPS insert error: " << e.what() << "\n";
    }
}

