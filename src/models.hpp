#pragma once

#include <string>
#include <vector>
#include <chrono>

// Core Item Record representing either a LOST or FOUND item
struct Item {
    int id = 0;
    std::string type;            // "LOST" or "FOUND"
    std::string name;            // Name / title of the item
    std::string category;        // e.g., Electronics, Cards, Books, Keys, Bags, Clothing, Other
    std::string description;     // Detailed physical description
    std::string colour;          // Primary and secondary colours
    std::string location;        // Campus location (e.g., Central Library, Lab 304, Cafeteria)
    std::string date;            // Date formatted as YYYY-MM-DD
    std::string contact_name;    // Reporter's name
    std::string contact_info;    // Phone number or email
    std::string student_id;      // Campus ID / Roll Number
    std::string storage_location;// Safe collection room (for Found items)
    std::string status;          // "OPEN", "IN_REVIEW", "MATCHED", "RETURNED", "CLOSED"
    std::string image_url;       // Image URL or uploaded file path
    std::string created_at;      // Timestamp of creation
    std::string department;      // Academic department (e.g. Computer Science, AI&DS)
    std::string claim_token;     // Tamper-proof token (e.g. DYPTC-REC-014-9A2F)
    std::string challenge_q;     // Ownership verification question
    std::string challenge_a;     // Ownership verification answer
};

// Detailed breakdown of the 6-factor weighted matching algorithm
struct MatchBreakdown {
    double total_score = 0.0;       // 0.0 to 100.0 %
    double category_score = 0.0;    // Weight: 25% max
    double name_score = 0.0;        // Weight: 25% max
    double colour_score = 0.0;      // Weight: 15% max
    double location_score = 0.0;    // Weight: 15% max
    double description_score = 0.0; // Weight: 10% max
    double date_score = 0.0;        // Weight: 10% max
    std::vector<std::string> reasons; // Explanations for why points were awarded
};

// A scored pair of Lost Item and Found Item
struct MatchResult {
    Item lost_item;
    Item found_item;
    MatchBreakdown breakdown;
};

// Comparator for std::priority_queue (Max-Heap by matching score)
struct MatchComparator {
    bool operator()(const MatchResult& a, const MatchResult& b) const {
        // Less-than operator ensures top() yields the item with the highest total_score
        return a.breakdown.total_score < b.breakdown.total_score;
    }
};

// Metrics captured during sorting demonstrations
struct SortMetrics {
    std::string algorithm_name;
    size_t item_count = 0;
    long long comparisons = 0;
    long long swaps_or_shifts = 0;
    long long duration_microseconds = 0;
};

// Metrics captured during search demonstrations
struct SearchMetrics {
    std::string algorithm_name;
    size_t total_items_checked = 0;
    long long comparisons = 0;
    long long duration_microseconds = 0;
    bool item_found = false;
    int found_index = -1;
};

// Dashboard KPI statistics
struct DashboardStats {
    int total_lost = 0;
    int total_found = 0;
    int total_matched = 0;
    int total_returned = 0;
    int total_active_cases = 0;
    int total_items = 0;
};

// ============================================================================
// ADVANCED FEATURES DATA MODELS (DYPTC System Extensions)
// ============================================================================

// 1. Live Benchmark Profiling Model
struct BenchmarkResult {
    std::string algorithm_name;
    std::string category;              // "Sorting", "Searching", "Graph / DP"
    std::string time_complexity_best;
    std::string time_complexity_avg;
    std::string time_complexity_worst;
    std::string space_complexity;
    long long execution_time_microseconds = 0;
    long long operations_counter = 0;
    bool is_stable = false;
};

struct FullBenchmarkSuite {
    std::vector<BenchmarkResult> sorting_benchmarks;
    std::vector<BenchmarkResult> searching_benchmarks;
    BenchmarkResult levenshtein_benchmark;
    BenchmarkResult priority_queue_benchmark;
    BenchmarkResult dijkstra_benchmark;
    BenchmarkResult trie_benchmark;
    size_t dataset_size = 0;
};

// 2. Department-Wise Honors Model
struct DepartmentHonors {
    std::string department_name;
    int items_reported = 0;
    int items_returned = 0;
    double honesty_score = 0.0;
};

// 3. Security Audit Log Model
struct AuditLogEntry {
    int id = 0;
    int item_id = 0;
    std::string action;        // "CREATED", "STATUS_CHANGE", "VERIFIED", "CLAIMED"
    std::string old_status;
    std::string new_status;
    std::string performed_by;  // Student roll number or Admin ID
    std::string timestamp;     // YYYY-MM-DD HH:MM:SS
    std::string remarks;
};

// 4. Official Printable Receipt Model
struct ReceiptData {
    std::string receipt_number; // e.g. "DYPTC-REC-2026-0042"
    Item item;
    std::string claimant_name;
    std::string claimant_student_id;
    std::string claimant_contact;
    std::string verification_code;
    std::string handover_officer;
    std::string handover_location;
    std::string issued_at;
};

// 5. Ownership Challenge-Response Model
struct OwnershipChallenge {
    int item_id = 0;
    std::string secret_question; // e.g. "What is the lock screen wallpaper or sticker?"
    std::string expected_answer; // lowercase normalized answer
    bool is_verified = false;
};

// 6. Campus Heatmap Zone Model
struct CampusZoneStat {
    std::string zone_name;
    std::string building_code;
    int lost_count = 0;
    int found_count = 0;
    int total_incidents = 0;
    std::string density_level; // "LOW", "MEDIUM", "HIGH"
};

// 7. Bloom Filter Diagnostic Model
struct BloomFilterStats {
    size_t capacity_bits = 1024;
    size_t set_bits_count = 0;
    double load_factor = 0.0;
    size_t hash_functions_k = 3;
};
