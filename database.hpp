#pragma once

#include "models.hpp"
#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <optional>
#include <mutex>

class Database {
private:
    sqlite3* db_ = nullptr;
    std::recursive_mutex db_mutex_;
    std::string db_path_;

public:
    Database() = default;

    ~Database() {
        close();
    }

    bool init(const std::string& dbPath = "data/lost_found.db") {
        std::lock_guard<std::recursive_mutex> lock(db_mutex_);
        db_path_ = dbPath;

        int rc = sqlite3_open(db_path_.c_str(), &db_);
        if (rc != SQLITE_OK) {
            std::cerr << "[Database Error] Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
            return false;
        }

        // Create items table with robust schema
        const char* createTableSql = R"(
            CREATE TABLE IF NOT EXISTS items (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                type TEXT NOT NULL,
                name TEXT NOT NULL,
                category TEXT NOT NULL,
                description TEXT NOT NULL,
                colour TEXT NOT NULL,
                location TEXT NOT NULL,
                date TEXT NOT NULL,
                contact_name TEXT NOT NULL,
                contact_info TEXT NOT NULL,
                student_id TEXT NOT NULL,
                storage_location TEXT DEFAULT '',
                status TEXT NOT NULL DEFAULT 'OPEN',
                image_url TEXT DEFAULT '',
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            );
            CREATE INDEX IF NOT EXISTS idx_items_type ON items(type);
            CREATE INDEX IF NOT EXISTS idx_items_category ON items(category);
            CREATE INDEX IF NOT EXISTS idx_items_status ON items(status);
        )";

        char* errMsg = nullptr;
        rc = sqlite3_exec(db_, createTableSql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "[Database Error] Table creation failed: " << (errMsg ? errMsg : "Unknown") << std::endl;
            sqlite3_free(errMsg);
            return false;
        }

        // Create audit_logs table
        const char* createAuditTableSql = R"(
            CREATE TABLE IF NOT EXISTS audit_logs (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                item_id INTEGER NOT NULL,
                action TEXT NOT NULL,
                old_status TEXT DEFAULT '',
                new_status TEXT NOT NULL,
                performed_by TEXT NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                remarks TEXT DEFAULT ''
            );
            CREATE INDEX IF NOT EXISTS idx_audit_item_id ON audit_logs(item_id);
        )";
        sqlite3_exec(db_, createAuditTableSql, nullptr, nullptr, nullptr);

        std::cout << "[Database] SQLite initialized successfully at: " << db_path_ << std::endl;
        return true;
    }

    static std::string generateToken(int id, const std::string& type, const std::string& date) {
        unsigned int hash = 2166136261u;
        std::string seed = std::to_string(id) + "|" + type + "|" + date + "|DYPTC_2026";
        for (char c : seed) {
            hash = (hash ^ static_cast<unsigned char>(c)) * 16777619u;
        }
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%04X", hash & 0xFFFF);
        std::string typeCode = (type == "FOUND" ? "FND" : "LST");
        std::string idStr = (id < 10 ? "0" : "") + std::to_string(id);
        return "DYPTC-" + typeCode + "-" + idStr + "-" + std::string(buf);
    }

    static void enrichItemDefaults(Item& item) {
        if (item.claim_token.empty()) {
            item.claim_token = generateToken(item.id, item.type, item.date);
        }
        if (item.department.empty() || item.department == "General") {
            std::string sid = item.student_id;
            std::transform(sid.begin(), sid.end(), sid.begin(), ::tolower);
            std::string loc = item.location;
            std::transform(loc.begin(), loc.end(), loc.begin(), ::tolower);
            if (sid.find("ce") != std::string::npos && sid.find("ece") == std::string::npos && sid.find("cse") == std::string::npos) item.department = "Computer Engineering (CE)";
            else if (sid.find("cs") != std::string::npos && sid.find("cse") == std::string::npos) item.department = "Computer Science (CS)";
            else if (sid.find("aids") != std::string::npos || sid.find("ai&ds") != std::string::npos || loc.find("data science") != std::string::npos) item.department = "AI & Data Science (AI&DS)";
            else if (sid.find("ece") != std::string::npos || sid.find("entc") != std::string::npos) item.department = "Electronics & Computer Engg. (ECE)";
            else if (sid.find("mca") != std::string::npos) item.department = "Master of Computer Applications (MCA)";
            else if (sid.find("arch") != std::string::npos || loc.find("architecture") != std::string::npos) item.department = "Architecture";
            else if (sid.find("daiml") != std::string::npos) item.department = "Diploma in AI & ML";
            else if (sid.find("cse") != std::string::npos || sid.find("dcse") != std::string::npos) item.department = "Diploma in CSE";
            else if (loc.find("computer") != std::string::npos) item.department = "Computer Engineering (CE)";
            else if (sid.find("staff") != std::string::npos || loc.find("security") != std::string::npos || loc.find("canteen") != std::string::npos) item.department = "Campus Administration";
            else item.department = "First Year Engineering (FE)";
        }
        if (item.challenge_q.empty() && item.type == "FOUND") {
            item.challenge_q = "Please state unique identification markings, password hint, or purchase receipt.";
        }
    }

    void close() {
        std::lock_guard<std::recursive_mutex> lock(db_mutex_);
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

    bool insertItem(Item& item) {
        std::lock_guard<std::recursive_mutex> lock(db_mutex_);
        if (!db_) return false;

        const char* sql = R"(
            INSERT INTO items (
                type, name, category, description, colour, location, 
                date, contact_name, contact_info, student_id, storage_location, 
                status, image_url
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
        )";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "[Database Error] Prepare failed: " << sqlite3_errmsg(db_) << std::endl;
            return false;
        }

        // Bind parameters safely to prevent SQL injection
        sqlite3_bind_text(stmt, 1, item.type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, item.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, item.category.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, item.description.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, item.colour.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, item.location.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, item.date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, item.contact_name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 9, item.contact_info.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 10, item.student_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 11, item.storage_location.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 12, item.status.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 13, item.image_url.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "[Database Error] Step failed: " << sqlite3_errmsg(db_) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        item.id = static_cast<int>(sqlite3_last_insert_rowid(db_));
        enrichItemDefaults(item);
        sqlite3_finalize(stmt);

        // Record initial audit trail entry
        logAudit(item.id, "REPORTED", "", item.status, item.student_id, "Item registered in DYPTC system");
        return true;
    }

    std::vector<Item> getAllItems() {
        std::lock_guard<std::recursive_mutex> lock(db_mutex_);
        std::vector<Item> results;
        if (!db_) return results;

        const char* sql = "SELECT id, type, name, category, description, colour, location, date, contact_name, contact_info, student_id, storage_location, status, image_url, created_at FROM items ORDER BY id DESC;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "[Database Error] Select failed: " << sqlite3_errmsg(db_) << std::endl;
            return results;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Item item;
            item.id = sqlite3_column_int(stmt, 0);
            item.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1) ? sqlite3_column_text(stmt, 1) : reinterpret_cast<const unsigned char*>(""));
            item.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2) ? sqlite3_column_text(stmt, 2) : reinterpret_cast<const unsigned char*>(""));
            item.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3) ? sqlite3_column_text(stmt, 3) : reinterpret_cast<const unsigned char*>(""));
            item.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4) ? sqlite3_column_text(stmt, 4) : reinterpret_cast<const unsigned char*>(""));
            item.colour = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5) ? sqlite3_column_text(stmt, 5) : reinterpret_cast<const unsigned char*>(""));
            item.location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6) ? sqlite3_column_text(stmt, 6) : reinterpret_cast<const unsigned char*>(""));
            item.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7) ? sqlite3_column_text(stmt, 7) : reinterpret_cast<const unsigned char*>(""));
            item.contact_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8) ? sqlite3_column_text(stmt, 8) : reinterpret_cast<const unsigned char*>(""));
            item.contact_info = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9) ? sqlite3_column_text(stmt, 9) : reinterpret_cast<const unsigned char*>(""));
            item.student_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10) ? sqlite3_column_text(stmt, 10) : reinterpret_cast<const unsigned char*>(""));
            item.storage_location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11) ? sqlite3_column_text(stmt, 11) : reinterpret_cast<const unsigned char*>(""));
            item.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12) ? sqlite3_column_text(stmt, 12) : reinterpret_cast<const unsigned char*>(""));
            item.image_url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13) ? sqlite3_column_text(stmt, 13) : reinterpret_cast<const unsigned char*>(""));
            item.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14) ? sqlite3_column_text(stmt, 14) : reinterpret_cast<const unsigned char*>(""));
            enrichItemDefaults(item);
            results.push_back(std::move(item));
        }

        sqlite3_finalize(stmt);
        return results;
    }

    std::optional<Item> getItemById(int id) {
        std::lock_guard<std::recursive_mutex> lock(db_mutex_);
        if (!db_) return std::nullopt;

        const char* sql = "SELECT id, type, name, category, description, colour, location, date, contact_name, contact_info, student_id, storage_location, status, image_url, created_at FROM items WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return std::nullopt;
        }

        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            Item item;
            item.id = sqlite3_column_int(stmt, 0);
            item.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1) ? sqlite3_column_text(stmt, 1) : reinterpret_cast<const unsigned char*>(""));
            item.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2) ? sqlite3_column_text(stmt, 2) : reinterpret_cast<const unsigned char*>(""));
            item.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3) ? sqlite3_column_text(stmt, 3) : reinterpret_cast<const unsigned char*>(""));
            item.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4) ? sqlite3_column_text(stmt, 4) : reinterpret_cast<const unsigned char*>(""));
            item.colour = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5) ? sqlite3_column_text(stmt, 5) : reinterpret_cast<const unsigned char*>(""));
            item.location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6) ? sqlite3_column_text(stmt, 6) : reinterpret_cast<const unsigned char*>(""));
            item.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7) ? sqlite3_column_text(stmt, 7) : reinterpret_cast<const unsigned char*>(""));
            item.contact_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8) ? sqlite3_column_text(stmt, 8) : reinterpret_cast<const unsigned char*>(""));
            item.contact_info = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9) ? sqlite3_column_text(stmt, 9) : reinterpret_cast<const unsigned char*>(""));
            item.student_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10) ? sqlite3_column_text(stmt, 10) : reinterpret_cast<const unsigned char*>(""));
            item.storage_location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11) ? sqlite3_column_text(stmt, 11) : reinterpret_cast<const unsigned char*>(""));
            item.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12) ? sqlite3_column_text(stmt, 12) : reinterpret_cast<const unsigned char*>(""));
            item.image_url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13) ? sqlite3_column_text(stmt, 13) : reinterpret_cast<const unsigned char*>(""));
            item.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14) ? sqlite3_column_text(stmt, 14) : reinterpret_cast<const unsigned char*>(""));
            enrichItemDefaults(item);

            sqlite3_finalize(stmt);
            return item;
        }

        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    bool logAudit(int itemId, const std::string& action, const std::string& oldStatus, const std::string& newStatus, const std::string& performedBy, const std::string& remarks) {
        std::lock_guard<std::recursive_mutex> lock(db_mutex_);
        if (!db_) return false;

        const char* sql = "INSERT INTO audit_logs (item_id, action, old_status, new_status, performed_by, remarks) VALUES (?, ?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_int(stmt, 1, itemId);
        sqlite3_bind_text(stmt, 2, action.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, oldStatus.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, newStatus.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, performedBy.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, remarks.c_str(), -1, SQLITE_TRANSIENT);

        bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return ok;
    }

    std::vector<AuditLogEntry> getAuditLogs(int limit = 50) {
        std::lock_guard<std::recursive_mutex> lock(db_mutex_);
        std::vector<AuditLogEntry> list;
        if (!db_) return list;

        std::string sql = "SELECT id, item_id, action, old_status, new_status, performed_by, timestamp, remarks FROM audit_logs ORDER BY id DESC LIMIT " + std::to_string(limit) + ";";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return list;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            AuditLogEntry e;
            e.id = sqlite3_column_int(stmt, 0);
            e.item_id = sqlite3_column_int(stmt, 1);
            e.action = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2) ? sqlite3_column_text(stmt, 2) : (const unsigned char*)"");
            e.old_status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3) ? sqlite3_column_text(stmt, 3) : (const unsigned char*)"");
            e.new_status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4) ? sqlite3_column_text(stmt, 4) : (const unsigned char*)"");
            e.performed_by = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5) ? sqlite3_column_text(stmt, 5) : (const unsigned char*)"");
            e.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6) ? sqlite3_column_text(stmt, 6) : (const unsigned char*)"");
            e.remarks = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7) ? sqlite3_column_text(stmt, 7) : (const unsigned char*)"");
            list.push_back(std::move(e));
        }
        sqlite3_finalize(stmt);
        return list;
    }

    std::vector<AuditLogEntry> getAuditLogsForItem(int itemId) {
        std::lock_guard<std::recursive_mutex> lock(db_mutex_);
        std::vector<AuditLogEntry> list;
        if (!db_) return list;

        const char* sql = "SELECT id, item_id, action, old_status, new_status, performed_by, timestamp, remarks FROM audit_logs WHERE item_id = ? ORDER BY id DESC;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return list;
        sqlite3_bind_int(stmt, 1, itemId);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            AuditLogEntry e;
            e.id = sqlite3_column_int(stmt, 0);
            e.item_id = sqlite3_column_int(stmt, 1);
            e.action = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2) ? sqlite3_column_text(stmt, 2) : (const unsigned char*)"");
            e.old_status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3) ? sqlite3_column_text(stmt, 3) : (const unsigned char*)"");
            e.new_status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4) ? sqlite3_column_text(stmt, 4) : (const unsigned char*)"");
            e.performed_by = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5) ? sqlite3_column_text(stmt, 5) : (const unsigned char*)"");
            e.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6) ? sqlite3_column_text(stmt, 6) : (const unsigned char*)"");
            e.remarks = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7) ? sqlite3_column_text(stmt, 7) : (const unsigned char*)"");
            list.push_back(std::move(e));
        }
        sqlite3_finalize(stmt);
        return list;
    }

    bool updateItemStatus(int id, const std::string& newStatus, const std::string& performedBy = "ADMIN_DESK", const std::string& remarks = "Status updated") {
        std::string oldStatus = "OPEN";
        {
            std::lock_guard<std::recursive_mutex> lock(db_mutex_);
            if (!db_) return false;

            const char* qSql = "SELECT status FROM items WHERE id = ?;";
            sqlite3_stmt* qStmt = nullptr;
            if (sqlite3_prepare_v2(db_, qSql, -1, &qStmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(qStmt, 1, id);
                if (sqlite3_step(qStmt) == SQLITE_ROW) {
                    oldStatus = reinterpret_cast<const char*>(sqlite3_column_text(qStmt, 0) ? sqlite3_column_text(qStmt, 0) : (const unsigned char*)"");
                }
                sqlite3_finalize(qStmt);
            }

            const char* sql = "UPDATE items SET status = ? WHERE id = ?;";
            sqlite3_stmt* stmt = nullptr;
            if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
                return false;
            }

            sqlite3_bind_text(stmt, 1, newStatus.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, id);

            bool success = (sqlite3_step(stmt) == SQLITE_DONE);
            sqlite3_finalize(stmt);
            if (!success) return false;
        }

        logAudit(id, "STATUS_CHANGE", oldStatus, newStatus, performedBy, remarks);
        return true;
    }

    int archiveOldItems(int daysThreshold = 90) {
        std::vector<Item> all = getAllItems();
        int count = 0;
        for (const auto& item : all) {
            if (item.status == "OPEN" && item.date.length() >= 10) {
                try {
                    int y = std::stoi(item.date.substr(0, 4));
                    int m = std::stoi(item.date.substr(5, 2));
                    int d = std::stoi(item.date.substr(8, 2));
                    int diff = (2026 - y) * 365 + (10 - m) * 30 + (20 - d);
                    if (diff >= daysThreshold) {
                        updateItemStatus(item.id, "ARCHIVED", "CAMPUS_ARCHIVE_CRON", "Exceeded 90-day retention policy; moved to custodial auction/donation review");
                        count++;
                    }
                } catch (...) {}
            }
        }
        return count;
    }

    bool deleteItem(int id) {
        std::lock_guard<std::recursive_mutex> lock(db_mutex_);
        if (!db_) return false;

        const char* sql = "DELETE FROM items WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_int(stmt, 1, id);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return success;
    }

    int getItemCount() {
        std::lock_guard<std::recursive_mutex> lock(db_mutex_);
        if (!db_) return 0;

        const char* sql = "SELECT COUNT(*) FROM items;";
        sqlite3_stmt* stmt = nullptr;
        int count = 0;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                count = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
        return count;
    }

    bool seedSampleData() {
        if (getItemCount() > 0) {
            return true; // Already populated
        }

        std::vector<Item> samples = {
            // Pair 1: HP Pavilion Laptop (Strong Match)
            {0, "LOST", "HP Pavilion 15 Silver Laptop", "Electronics", 
             "15.6 inch silver laptop with Intel i7 sticker on palmrest and small scratch on lid", 
             "Silver", "DYPTC Central Library - 2nd Floor Reading Hall", "2026-10-14", 
             "Aryan Sharma", "aryan.sharma@dypatiltcs.com / +91 9876543210", "DYPTC-24-CS-014", "", "OPEN", 
             "/data/uploads/laptop.svg", ""},
             
            {0, "FOUND", "HP Pavilion Laptop (Silver)", "Electronics", 
             "Silver HP Pavilion laptop found plugged in under desk 14 with black power adapter", 
             "Silver", "DYPTC Central Library", "2026-10-14", 
             "Library Security Desk", "security.library@dypatiltcs.com", "STAFF-082", 
             "Central Library Helpdesk Lockbox 03", "OPEN", 
             "/data/uploads/laptop.svg", ""},

            // Pair 2: AirPods Pro (Strong Match)
            {0, "LOST", "Apple AirPods Pro (2nd Gen)", "Electronics", 
             "White wireless earbuds in white MagSafe case with red silicone cover and carabiner", 
             "White / Red", "AI & Data Science Lab 204 - DYPTC", "2026-10-15", 
             "Priya Patel", "priya.patel@dypatiltcs.com / +91 9811223344", "DYPTC-24-AIDS-045", "", "OPEN", 
             "/data/uploads/earbuds.svg", ""},

            {0, "FOUND", "White Earbuds in Red Silicone Case", "Electronics", 
             "Apple wireless earbuds inside protective red silicone sleeve found on bench near Lab 3", 
             "White / Red", "AI & Data Science Lab Corridor - DYPTC", "2026-10-15", 
             "Lab Assistant Rahul", "lab.support@dypatiltcs.com", "STAFF-119", 
             "Main IT Support Office Room 104", "OPEN", 
             "/data/uploads/earbuds.svg", ""},

            // Pair 3: Campus Student ID Card
            {0, "LOST", "Student ID Card - Rohan Verma", "ID & Cards", 
             "Smart campus card bearing roll number BT24ECE089 with blue university lanyard", 
             "Blue / White", "DYPTC Varale Campus Canteen & Food Court", "2026-10-16", 
             "Rohan Verma", "rohan.v@dypatiltcs.com / +91 9988776655", "DYPTC-24-ECE-089", "", "OPEN", 
             "/data/uploads/idcard.svg", ""},

            {0, "FOUND", "Student ID Card on Blue Lanyard", "ID & Cards", 
             "Campus identity card for Rohan Verma found on dining table in canteen", 
             "Blue / White", "DYPTC Canteen", "2026-10-16", 
             "Canteen Supervisor", "canteen@dypatiltcs.com", "STAFF-041", 
             "Campus Security Control Room", "OPEN", 
             "/data/uploads/idcard.svg", ""},

            // Pair 4: Keys with Hostel Keychain
            {0, "LOST", "Room Keys with Maroon Lanyard", "Keys & Accessories", 
             "Set of 3 Godrej keys attached to a maroon leather tag stamped DYPTC Varale Campus Boys Hostel Block B Room 212", 
             "Maroon / Brass", "DYPTC Sports Ground & Indoor Badminton Court", "2026-10-13", 
             "Devendra Rao", "d.rao@dypatiltcs.com / +91 9765432109", "DYPTC-24-MCA-022", "", "OPEN", 
             "/data/uploads/keys.svg", ""},

            {0, "FOUND", "Set of 3 Keys with Leather Tag", "Keys & Accessories", 
             "Three door keys on maroon keychain found beside indoor badminton court", 
             "Maroon / Silver", "DYPTC Sports Ground", "2026-10-13", 
             "Gym Instructor", "sports@dypatiltcs.com", "STAFF-203", 
             "Sports Complex Admin Desk", "OPEN", 
             "/data/uploads/keys.svg", ""},

            // Pair 5: Scientific Calculator
            {0, "LOST", "Casio FX-991CW Calculator", "Books & Stationery", 
             "Black scientific engineering calculator with white cover and name initials KV engraved", 
             "Black", "Architecture Design Studio & Lecture Hall LH-2", "2026-10-17", 
             "Kavita Varma", "kavita.v@dypatiltcs.com / +91 9845012345", "DYPTC-24-ARCH-012", "", "OPEN", 
             "/data/uploads/calculator.svg", ""},

            {0, "FOUND", "Casio Scientific Calculator", "Books & Stationery", 
             "Casio FX series advanced calculator left behind on second row desk in LH-2", 
             "Black", "DYPTC Architecture Building LH-2", "2026-10-17", 
             "Janitor Murthy", "estate@dypatiltcs.com", "STAFF-007", 
             "Academic Section Counter 4", "OPEN", 
             "/data/uploads/calculator.svg", ""},

            // Additional items (Unmatched & Resolved cases for testing)
            {0, "LOST", "North Face Navy Blue Backpack", "Bags & Backpacks", 
             "Dark navy water-resistant backpack containing DSA laboratory notebook and pencil case", 
             "Navy Blue", "Auditorium Main Gate", "2026-10-12", 
             "Siddharth Sen", "s.sen@dypatiltcs.com / +91 9123456780", "BT24CE099", "", "OPEN", 
             "/data/uploads/bag.svg", ""},

            {0, "FOUND", "Hydro Flask 32oz Insulated Bottle", "Other", 
             "Stainless steel olive green vacuum flask with stickers including GitHub Octocat", 
             "Olive Green", "Campus Green Lawn / Amphitheatre", "2026-10-16", 
             "Aditi Bose", "aditi.bose@dypatiltcs.com", "BT24CS019", 
             "Student Affairs Office", "OPEN", 
             "/data/uploads/bottle.svg", ""},

            {0, "LOST", "Casio G-Shock Matte Black Watch", "Clothing & Wearables", 
             "Digital sports watch with resin strap lost during evening football practice", 
             "Matte Black", "Football Ground", "2026-10-10", 
             "Farhan Akhtar", "farhan.a@dypatiltcs.com / +91 9871203948", "BT24ECE034", "", "RETURNED", 
             "/data/uploads/watch.svg", ""},

            {0, "FOUND", "Black Digital Sports Watch", "Clothing & Wearables", 
             "Black rugged shock-resistant watch found on player bench beside goalpost", 
             "Black", "Football Ground Bench", "2026-10-10", 
             "Coach Nair", "sports.dept@dypatiltcs.com", "STAFF-155", 
             "Returned to Owner Farhan Akhtar", "RETURNED", 
             "/data/uploads/watch.svg", ""}
        };

        for (auto& item : samples) {
            insertItem(item);
        }

        std::cout << "[Database] Successfully seeded " << samples.size() << " realistic campus records." << std::endl;
        return true;
    }
};
