#pragma once
// ================================================================
//  Axion Digitaverse - aipfs.h  [Phase 1]
//  Axion InterPlanetary File System
//
//  Phase 1 upgrade:
//    Before: in-memory only (data lost on shutdown)
//    After:  KVStore-backed persistence (data survives restarts)
//            Same API - drop-in upgrade
// ================================================================

#include "sha256.h"
#include "kvstore.h"
#include <unordered_set>
#include <optional>
#include <mutex>

namespace axion {

inline std::string make_cid(const std::string& data) {
    return "axion://" + sha256(data);
}

class AIPFS {
public:
    static constexpr size_t CHUNK_SIZE   = 256 * 1024;
    static constexpr int    MIN_REPLICAS = 3;

    explicit AIPFS(const std::string& data_dir = "./axion_data")
        : storage_(data_dir)
        , chunks_db_(storage_.open("aipfs_chunks"))
        , meta_db_(storage_.open("aipfs_meta"))
        , pins_db_(storage_.open("aipfs_pins"))
    {
        // Count existing items
        size_t existing = meta_db_.size();
        std::cout << "[AIPFS] Storage initialized"
                  << "  existing_items=" << existing << "\n";
    }

    // Store data, return CID
    std::string put(const std::string& data,
                    const std::string& owner = "") {
        std::lock_guard<std::mutex> lock(mtx_);
        std::string cid = make_cid(data);

        // Already stored?
        if (meta_db_.exists(cid)) {
            if (!owner.empty()) pin_nolock(cid, owner);
            return cid;
        }

        // Chunk and store
        std::vector<std::string> chunk_cids;
        if (data.size() > CHUNK_SIZE) {
            for (size_t off = 0; off < data.size(); off += CHUNK_SIZE) {
                std::string chunk = data.substr(off,
                    std::min(CHUNK_SIZE, data.size() - off));
                chunk_cids.push_back(store_chunk(chunk));
            }
        } else {
            chunk_cids.push_back(store_chunk(data));
        }

        // Store metadata
        std::string meta = std::to_string(data.size()) + "|" + owner;
        for (const auto& cc : chunk_cids) meta += "|" + cc;
        meta_db_.put(cid, meta);

        if (!owner.empty()) pin_nolock(cid, owner);

        std::cout << "[AIPFS] Stored  cid=" << cid.substr(8,16)
                  << "...  size=" << data.size()
                  << "B  chunks=" << chunk_cids.size() << "\n";
        return cid;
    }

    // Retrieve data by CID
    std::optional<std::string> get(const std::string& cid) const {
        std::lock_guard<std::mutex> lock(mtx_);
        std::string meta;
        if (!meta_db_.get(cid, meta)) return std::nullopt;

        // Parse metadata to get chunk CIDs
        std::vector<std::string> parts;
        std::istringstream iss(meta);
        std::string tok;
        while (std::getline(iss, tok, '|')) parts.push_back(tok);
        if (parts.size() < 3) return std::nullopt;

        std::string result;
        for (size_t i = 2; i < parts.size(); ++i) {
            std::string chunk;
            if (!chunks_db_.get(parts[i], chunk)) return std::nullopt;
            result += chunk;
        }
        return result;
    }

    void pin(const std::string& cid, const std::string& pinner) {
        std::lock_guard<std::mutex> lock(mtx_);
        pin_nolock(cid, pinner);
    }

    void unpin(const std::string& cid, const std::string& pinner) {
        std::lock_guard<std::mutex> lock(mtx_);
        std::string pinners_str;
        if (!pins_db_.get(cid, pinners_str)) return;
        // Remove pinner from pipe-separated list
        std::string new_str;
        std::istringstream iss(pinners_str);
        std::string tok;
        while (std::getline(iss, tok, '|'))
            if (tok != pinner) new_str += tok + "|";
        pins_db_.put(cid, new_str);
    }

    bool is_pinned(const std::string& cid) const {
        std::lock_guard<std::mutex> lock(mtx_);
        std::string val;
        if (!pins_db_.get(cid, val)) return false;
        return !val.empty();
    }

    size_t item_count() const { return meta_db_.size(); }

    void print_stats() const {
        std::cout << "\n-- AIPFS Stats -------------------------\n";
        std::cout << "  Items stored : " << meta_db_.size() << "\n";
        std::cout << "  Chunk store  : " << chunks_db_.size() << " chunks\n";
        std::cout << "  Pins         : " << pins_db_.size() << "\n";
        std::cout << "  Data dir     : " << storage_.data_dir() << "\n";
    }

private:
    mutable std::mutex mtx_;
    StorageManager     storage_;
    KVStore&           chunks_db_;
    KVStore&           meta_db_;
    KVStore&           pins_db_;
    size_t             rr_index_ = 0;

    std::string store_chunk(const std::string& data) {
        std::string cid = "axion://" + sha256(data);
        if (!chunks_db_.exists(cid))
            chunks_db_.put(cid, data);
        return cid;
    }

    void pin_nolock(const std::string& cid, const std::string& pinner) {
        std::string existing;
        pins_db_.get(cid, existing);
        // Only add if not already there
        if (existing.find(pinner) == std::string::npos)
            pins_db_.put(cid, existing + pinner + "|");
    }
};

} // namespace axion
