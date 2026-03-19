#pragma once
// ================================================================
//  Axion Digitaverse - kvstore.h  [Phase 1]
//  Persistent Key-Value Store
//
//  A file-backed key-value store with the same API as RocksDB.
//  Swap to RocksDB with zero code changes in chain.h / aipfs.h
//  by replacing this #include with the RocksDB headers.
//
//  Format on disk:
//    One file per "column family" (database name).
//    File: simple binary log — each entry is:
//      [4-byte key_len][key_bytes][4-byte val_len][val_bytes]
//    On open: replay entire file into memory map.
//    On put: append to file + update memory map.
//    On delete: write tombstone (val_len = 0xFFFFFFFF).
//
//  Performance: adequate for testnet (thousands of blocks).
//  For mainnet: swap in RocksDB for LSM-tree performance.
// ================================================================

#include "types.h"
#include <fstream>
#include <unordered_map>
#include <mutex>
#include <filesystem>

namespace axion {

class KVStore {
public:
    // Open (or create) a store at the given file path
    explicit KVStore(const std::string& path) : path_(path) {
        load_from_disk();
        std::cout << "[KVStore] Opened '" << path_
                  << "'  entries=" << map_.size() << "\n";
    }

    ~KVStore() { flush(); }

    // Put a key-value pair (persists immediately)
    bool put(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(mtx_);
        map_[key] = value;
        append_to_disk(key, value, false);
        return true;
    }

    // Get a value by key. Returns empty string if not found.
    bool get(const std::string& key, std::string& value) const {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = map_.find(key);
        if (it == map_.end()) return false;
        value = it->second;
        return true;
    }

    // Delete a key
    bool del(const std::string& key) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!map_.count(key)) return false;
        map_.erase(key);
        append_to_disk(key, "", true);
        return true;
    }

    bool exists(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mtx_);
        return map_.count(key) > 0;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return map_.size();
    }

    // Iterate over all keys with a given prefix
    std::vector<std::string> keys_with_prefix(const std::string& prefix) const {
        std::lock_guard<std::mutex> lock(mtx_);
        std::vector<std::string> result;
        for (const auto& [k, v] : map_)
            if (k.substr(0, prefix.size()) == prefix)
                result.push_back(k);
        return result;
    }

    // Force flush (called in destructor too)
    void flush() {
        std::lock_guard<std::mutex> lock(mtx_);
        // Already append-logged — nothing extra to do
    }

    // Compact: rewrite file without tombstones (reduces file size)
    void compact() {
        std::lock_guard<std::mutex> lock(mtx_);
        std::ofstream out(path_, std::ios::binary | std::ios::trunc);
        if (!out) return;
        for (const auto& [k, v] : map_)
            write_entry(out, k, v, false);
    }

    const std::string& path() const { return path_; }

private:
    std::string                              path_;
    mutable std::mutex                       mtx_;
    std::unordered_map<std::string, std::string> map_;

    static constexpr uint32_t TOMBSTONE = 0xFFFFFFFF;

    void write_uint32(std::ostream& out, uint32_t v) {
        out.write(reinterpret_cast<const char*>(&v), 4);
    }

    uint32_t read_uint32(std::istream& in) {
        uint32_t v = 0;
        in.read(reinterpret_cast<char*>(&v), 4);
        return v;
    }

    void write_entry(std::ostream& out,
                     const std::string& key,
                     const std::string& val,
                     bool is_delete) {
        uint32_t klen = static_cast<uint32_t>(key.size());
        uint32_t vlen = is_delete ? TOMBSTONE
                                  : static_cast<uint32_t>(val.size());
        write_uint32(out, klen);
        out.write(key.data(), klen);
        write_uint32(out, vlen);
        if (!is_delete) out.write(val.data(), vlen);
    }

    void append_to_disk(const std::string& key,
                        const std::string& val,
                        bool is_delete) {
        std::ofstream out(path_, std::ios::binary | std::ios::app);
        if (!out) return;
        write_entry(out, key, val, is_delete);
    }

    void load_from_disk() {
        std::ifstream in(path_, std::ios::binary);
        if (!in) return;  // file doesn't exist yet — fresh start
        while (in.peek() != EOF) {
            uint32_t klen = read_uint32(in);
            if (!in) break;
            std::string key(klen, '\0');
            in.read(&key[0], klen);
            uint32_t vlen = read_uint32(in);
            if (!in) break;
            if (vlen == TOMBSTONE) {
                map_.erase(key);
            } else {
                std::string val(vlen, '\0');
                in.read(&val[0], vlen);
                map_[key] = val;
            }
        }
    }
};

// ── StorageManager: manages multiple named KVStores ───────
// Mirrors RocksDB's column family concept.
class StorageManager {
public:
    explicit StorageManager(const std::string& data_dir)
        : data_dir_(data_dir)
    {
        // Create data directory if it doesn't exist
        try {
            std::filesystem::create_directories(data_dir);
        } catch (...) {
            // May fail if filesystem not available — in-memory fallback
        }
        std::cout << "[Storage] Data directory: " << data_dir << "\n";
    }

    // Open or create a named store
    KVStore& open(const std::string& name) {
        auto it = stores_.find(name);
        if (it != stores_.end()) return *it->second;
        std::string path = data_dir_ + "/" + name + ".axdb";
        stores_[name] = std::make_unique<KVStore>(path);
        return *stores_[name];
    }

    void compact_all() {
        for (auto& [name, store] : stores_)
            store->compact();
    }

    const std::string& data_dir() const { return data_dir_; }

private:
    std::string                                          data_dir_;
    std::unordered_map<std::string,
        std::unique_ptr<KVStore>>                        stores_;
};

} // namespace axion
