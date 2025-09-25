// inside process_frame after pm is available...

/*
More per-key decoders (additions to 

Add or replace the key-processing loop with these clearer decoders. This shows explicitly where data is decoded and stored.
*/


// small helper lambdas
auto bytes_to_uint32 = [](const std::vector<uint8_t>& b, size_t off) -> uint32_t {
    return uint32_t(b[off]) | (uint32_t(b[off+1])<<8) | (uint32_t(b[off+2])<<16) | (uint32_t(b[off+3])<<24);
};
auto bytes_to_uint16 = [](const std::vector<uint8_t>& b, size_t off) -> uint16_t {
    return uint16_t(b[off]) | (uint16_t(b[off+1])<<8);
};

for (const auto& kv : pm.keys) {
    // store raw (generic)
    db_.store_raw_key(device_imei, pm.command, kv.key_id, kv.value, pm.seq_id);

    // Key-specific decoding
    switch (kv.key_id) {
    case 0x01: { // Device ID / IMEI (ASCII)
        // value usually ASCII digits
        std::string imei(kv.value.begin(), kv.value.end());
        if (!imei.empty() && imei != device_imei) {
            device_imei = imei;
            // also upsert into registry (register or update last_seen)
            registry_.upsert_device(device_imei, socket_.remote_endpoint().address().to_string());
        }
        break;
    }
    case 0x10: { // Heartbeat
        // per spec: contains timestamp (4 bytes), status bytes, etc.
        if (kv.value.size() >= 4) {
            uint32_t ts = bytes_to_uint32(kv.value, 0);
            // store heartbeat raw for now
            // example: db_.store_heartbeat(device_imei, ts, pm.seq_id);
        }
        break;
    }
    case 0x20: { // GPS
        // per spec layout: lat(4, signed int, 1e7), lon(4), speed(2), direction(2), timestamp(4), ...
        if (kv.value.size() >= 14) {
            int32_t lat_i = int32_t(bytes_to_uint32(kv.value, 0));
            int32_t lon_i = int32_t(bytes_to_uint32(kv.value, 4));
            double lat = lat_i / 1e7;
            double lon = lon_i / 1e7;
            uint16_t speed = bytes_to_uint16(kv.value, 8);
            uint16_t heading = bytes_to_uint16(kv.value, 10);
            uint32_t ts = bytes_to_uint32(kv.value, 12);
            db_.store_gps(device_imei, ts, lat, lon, pm.seq_id);
        }
        break;
    }
    case 0x24: { // General Data (example fields)
        // decode according to doc
        break;
    }
    // add other cases here (battery, call record, beacon list)...
    default:
        // unknown key: already stored raw. Good for future decoders.
        break;
    }
}

