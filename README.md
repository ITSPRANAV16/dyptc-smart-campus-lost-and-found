# Dr. D. Y. Patil Technical Campus (DYPTC) – Lost & Found Matching System
> **Campus:** Dr. D. Y. Patil Technical Campus, Talegaon-Varale, Pune &bull; **DTE Code:** 6991  
> **Affiliation:** Savitribai Phule Pune University (SPPU) &bull; **Approval:** AICTE New Delhi  
> **Federation:** D Y Patil Academic Education Excellence Federation (DYPAEEF)

A high-performance, intelligent **Campus Lost & Found Matching System** custom-built for **Dr. D. Y. Patil Technical Campus (DYPTC)** using strictly **C++17**, the **Crow C++ microframework**, **SQLite3**, and pure **HTML5 & CSS3** (absolutely **zero JavaScript**).

Designed for engineering (CE, CS, AI&DS, ECE, FE), MCA, and Architecture students/staff to automate the recovery of lost belongings, calculators, laptops, and identity cards using algorithmic matching and data structures.

---

## 1. Strict Technology Compliance

| Layer | Technology | Rules & Boundaries |
|---|---|---|
| **Frontend** | Pure HTML5 & CSS3 | **0% JavaScript**. No `<script>` tags, no JS libraries, no inline handlers (`onclick`, etc.). Pure CSS animations, responsive grids, and server-rendered HTML. |
| **Backend & Logic** | Modern C++17 | 100% of routing, form processing, data validation, algorithms, and rendering logic are implemented in C++. |
| **Web Server** | Crow C++ Framework | High-throughput, multi-threaded HTTP server running on `http://localhost:18080`. |
| **Persistence** | SQLite3 Database | ACID-compliant relational storage (`data/lost_found.db`) with prepared statements to prevent SQL injection. |
| **DSA Engine** | Native C++ Standard Library & Custom Algorithms | Dynamic arrays, hash tables, max-heap priority queues, string distance DP, and sorting/searching routines. |

---

## 2. System Architecture & Pages

```
                       +---------------------------------------+
                       |   Browser Frontend (HTML5 & CSS3)     |
                       |       (Zero JavaScript Used)          |
                       +---------------------------------------+
                                          |
                                   HTTP GET / POST
                                          |
                                          v
                       +---------------------------------------+
                       |       Crow C++ Web Server             |
                       |      (Port: localhost:18080)          |
                       +---------------------------------------+
                                          |
             +----------------------------+----------------------------+
             |                                                         |
             v                                                         v
+-----------------------------+                           +-----------------------------+
|    DSA Engine (C++17)       |                           |   SQLite3 Persistence       |
| - std::vector cache         |                           | - Items Schema & Indexes    |
| - std::unordered_map Index  |                           | - Prepared Statements       |
| - std::priority_queue Heap  |                           | - Thread-safe DB access     |
| - Levenshtein & Jaccard DP  |                           | - data/lost_found.db        |
| - Merge / Insert / Bubble   |                           +-----------------------------+
| - Binary & Linear Search    |
+-----------------------------+
```

### Complete Web Pages Included:

1. **Home / Dashboard (`/`):** Summary metrics cards (Total Lost, Found, Matched, Returned), live DSA engine status banner, **Live High-Confidence Matches Showcase Feed** (computed in real-time), **Quick Category Explorer Pills** (1-click navigation), and recent campus reports.
2. **Report Lost Item (`/lost`):** Student-tailored 2-column layout with helpful submission tips, recovery guidelines, and a spacious 3-step form with complete server-side input persistence.
3. **Report Found Item (`/found`):** Submission form for discovered items with campus collection guidance, safe storage custody registration, and input preservation.
4. **Browse All Items (`/items`):** Catalog featuring an **Integrated Search Field**, **Segmented Status Filter Tabs** (`All`, `Open`, `Matched`, `Returned` with dynamic counts), and an interactive **DSA Sorting Algorithm Selector** (Merge Sort vs Bubble Sort vs Insertion Sort with live execution metrics).
5. **Search Items (`/search`):** Search interface demonstrating **Linear Search** ($O(N)$ sequential scan) vs **Binary Search** ($O(\log N)$ divide-and-conquer on sorted items) with live comparison and microsecond timing metrics.
6. **Matching Engine (`/matches`):** Real-time cross-evaluation ranked via `std::priority_queue` (Max-Heap), featuring an **Interactive Match Score Threshold Filter** ($\ge 25\%, \ge 40\%, \ge 60\%, \ge 75\%$) and **Transparent 6-Factor Algorithmic Score Breakdown Grids** (`Category 25%`, `Name 25%`, `Colour 15%`, `Location 15%`, `Tokens 10%`, `Date 10%`).
7. **Item Details (`/item?id=...`):** Full metadata view, privacy-protected contact masking (emails and phone numbers masked for regular students), custody handover room, and live potential matches.
8. **Admin Dashboard (`/admin`):** Password-protected portal (default key: `admin123`) with **Multi-Parameter Search & Filter Toolbar** (search by roll number, name, phone, item ID), direct database status modifier, record deletion, sample data seeding, and **RFC 4180 CSV Data Export (`/admin/export.csv`)** for administrative audits.

---

## 3. Data Structures & Algorithms (DSA) Implementation

This system actively integrates eight foundational Computer Science data structures and algorithms:

### 1. `std::vector<Item>` (Dynamic Array)
- **Role:** Primary in-memory cache for all items loaded from SQLite.
- **Complexity:** $O(1)$ random access, amortized $O(1)$ insertions.
- **Implementation:** Located in `models.hpp` and `dsa_engine.hpp`.

### 2. `std::unordered_map<std::string, std::vector<int>>` (Hash Table)
- **Role:** Inverted index mapping categories to item IDs for instant subset lookups.
- **Complexity:** $O(1)$ average time complexity for hashing and bucket lookup.
- **Implementation:** Built via `dsa::buildCategoryIndex()` in `dsa_engine.hpp`.

### 3. Linear Search vs. Binary Search
- **Linear Search:** Sequentially inspects each element across fields (name, description, category, location).
  - *Time Complexity:* $O(N)$
- **Binary Search:** Operates on a sorted vector (pre-sorted by `name` using Merge Sort). Repeatedly halves the search space ($[low, high]$) to locate matches.
  - *Time Complexity:* $O(\log N)$
- *Interactive Demonstration:* Available on `/search`. The UI displays elements scanned, comparison counts, and execution duration in microseconds ($\mu\text{s}$).

### 4. Sorting Demonstrations: Merge Sort, Bubble Sort, & Insertion Sort
- **Merge Sort:** Recursive divide-and-conquer algorithm with guaranteed $O(N \log N)$ performance and stability.
- **Bubble Sort:** Adjacent element swapping with an optimized early-termination flag (`swapped`). Worst/Average: $O(N^2)$, Best: $O(N)$.
- **Insertion Sort:** Incremental sorting by shifting larger elements in the sorted prefix. Worst/Average: $O(N^2)$, Best: $O(N)$.
- *Interactive Demonstration:* Selectable on `/items`. Displays live runtime metrics:
  ```
  Merge Sort: 14 items sorted in 547 µs (53 comparisons, 54 swaps/shifts)
  Bubble Sort: 14 items sorted in 1359 µs (88 comparisons, 57 swaps/shifts)
  ```

### 5. String Similarity Algorithms
- **Dynamic Programming Levenshtein Distance:** Computes the minimal single-character edits (insertions, deletions, substitutions) to transform string $A$ into string $B$.
  $$\text{Levenshtein Ratio} = 1.0 - \frac{\text{Distance}}{\max(\text{len}(A), \text{len}(B))}$$
- **Jaccard Token Similarity:** Tokenizes text into distinct lowercase words, computing intersection over union:
  $$J(A, B) = \frac{|A \cap B|}{|A \cup B|}$$
- **Hybrid Matcher:** Combines Jaccard token overlap (60%) with Levenshtein edit distance (40%) to handle word reordering (e.g., *"HP Pavilion Laptop"* vs *"Laptop HP Pavilion"*).

### 6. `std::priority_queue` (Max-Heap Match Ranking)
- **Role:** Extracts top-scoring match candidates in descending order of confidence score.
- **Comparator:** `MatchComparator` compares `a.breakdown.total_score < b.breakdown.total_score`.
- **Complexity:** Insertion $O(\log K)$, extraction $O(\log K)$.

---

## 4. Weighted Matching Algorithm (100% Breakdown)

When a lost item is evaluated against found items, the matching engine calculates a normalized score from $0\%$ to $100\%$:

| Criterion | Max Weight | Method & Justification |
|---|:---:|---|
| **Category Match** | **25%** | Exact category equality (e.g. both are *Electronics*). |
| **Item Name Similarity** | **25%** | Hybrid Levenshtein DP + Jaccard token overlap. |
| **Colour Similarity** | **15%** | Primary/secondary colour comparison. |
| **Campus Location** | **15%** | Campus zone string similarity (e.g. *Central Library*). |
| **Description Keywords**| **10%** | Jaccard token intersection of specific identifiers. |
| **Date Validity** | **10%** | Chronological check (found date on or after lost date). |
| **TOTAL** | **100%** | **Ranked via Max-Heap Priority Queue** |

> **Notice:** Algorithmic match suggestions are automated aids and do not constitute proof of ownership. Physical identity verification is required at the collection office.

---

## 5. VS Code & Project Structure

```
COURSE_PROJECT/
├── .vscode/
│   ├── tasks.json             # Build and run tasks (Ctrl+Shift+B)
│   ├── launch.json            # Debugging configuration (F5)
│   └── c_cpp_properties.json  # C++ IntelliSense paths
├── include/
│   └── crow_all.h             # Crow C++ Single Header Framework
├── data/
│   ├── lost_found.db          # Persistent SQLite3 Database
│   └── uploads/               # Vector SVG illustrations for items
│       ├── laptop.svg
│       ├── earbuds.svg
│       ├── idcard.svg
│       ├── keys.svg
│       ├── calculator.svg
│       ├── bag.svg
│       ├── bottle.svg
│       ├── watch.svg
│       └── item_default.svg
├── models.hpp                 # Data structures, Item, Metrics, Comparator
├── database.hpp               # SQLite3 interface with prepared statements
├── dsa_engine.hpp             # All 8 DSA algorithms & matching engine
├── html_renderer.hpp          # XSS escaping, URL decoding, UI generators
├── main.cpp                   # Crow server, routes, and controllers
├── index.html                 # Dashboard template
├── lost.html                  # Report Lost Item template
├── found.html                 # Report Found Item template
├── items.html                 # Browse catalog template with DSA sort
├── search.html                # Search template with Linear vs Binary search
├── matches.html               # Match engine template with priority queue
├── details.html               # Item details template with auto-matches
├── admin.html                 # Admin dashboard template with management table
├── style.css                  # Modern, responsive CSS3 design system
└── README.md                  # Comprehensive project documentation
```

---

## 6. Build and Run Instructions

### Prerequisites
- **Compiler:** GCC / G++ with C++17 support (e.g., MSYS2 UCRT64 / MinGW-w64 with `g++`).
- **Operating System:** Windows 10/11 (or Linux / macOS with equivalent compiler).
- **Libraries:** SQLite3 (`sqlite3.h` and `libsqlite3.a`), Winsock (`-lws2_32 -lmswsock` on Windows).

### Compilation Commands

Open a terminal (PowerShell or Command Prompt) in the project folder and execute:

```powershell
g++ -std=c++17 -O2 -Iinclude main.cpp -lsqlite3 -lws2_32 -lmswsock -o smart_campus.exe
```

### Running the Application

```powershell
.\smart_campus.exe
```

Expected startup output:
```
[Database] SQLite initialized successfully at: data/lost_found.db
[Database] Successfully seeded 14 realistic campus records.

======================================================
  Smart Campus Lost & Found Matching System (C++17)   
  Running at: http://localhost:18080                  
  Technology: Crow C++ + SQLite3 + Pure HTML5/CSS3     
  DSA Engine: Priority Queue, Merge/Bubble/Insertion, 
              Levenshtein & Jaccard, Linear & Binary  
======================================================

(INFO) Crow/master server is running at http://0.0.0.0:18080 using 4 threads
```

### Accessing the Web Application
Open your web browser and navigate to:
```
http://localhost:18080
```

---

## 7. Sample Testing Walkthrough

1. **View Dashboard (`/`):** View KPI cards for 14 pre-seeded campus belongings (HP laptops, AirPods Pro, ID cards, hostel keys, Casio calculators, etc.).
2. **Sort Demonstration (`/items`):** 
   - Choose `Sort Field: Item Name` and `DSA Algorithm: Bubble Sort (O(N²))`. Click **Apply & Sort**.
   - Note the metric banner displaying comparison and swap counts.
   - Switch to `Merge Sort (O(N log N))` and compare the efficiency.
3. **Search Demonstration (`/search`):**
   - Search for `laptop` using **Linear Search**. Notice the sequential comparison count.
   - Switch to **Binary Search**. Observe the reduced comparison steps.
4. **Algorithmic Matching (`/matches`):**
   - View automatically computed candidate pairs ranked by `std::priority_queue`.
   - Inspect the score progress bar and the breakdown of matching reasons.
5. **Report a New Lost Item (`/lost`):**
   - Submit a new item (e.g. *"Dell XPS Laptop"*, *Electronics*, *Silver*, *Computer Lab 2*).
   - Upon submission, the C++ engine saves the record to SQLite and redirects to `/matches?id=<new_id>`, displaying potential found matches.
6. **Admin Dashboard (`/admin`):**
   - View unmasked contact registers.
   - Update item status to `MATCHED` or `RETURNED`.
   - Verify that data changes persist across server restarts.

---

## 8. Troubleshooting

| Issue | Resolution |
|---|---|
| **Port 18080 already in use** | Ensure another instance of `smart_campus.exe` is not running. Kill existing instances via Task Manager or `Stop-Process -Name smart_campus`. |
| **`sqlite3.h` not found** | Verify MSYS2 UCRT64 include path is present in `tasks.json` / `-I` flag. In MSYS2, run `pacman -S mingw-w64-ucrt-x86_64-sqlite3`. |
| **Winsock linker errors (`WSAStartup`)** | Make sure `-lws2_32 -lmswsock` flags are included at the end of the `g++` compile command. |
| **Data reset** | To start with a fresh database, delete `data/lost_found.db`. The server will automatically recreate and re-seed it on next launch. |

---

## 9. Repository Structure

```
COURSE_PROJECT/
├── docs/                             # Academic Reports & Documentation
│   ├── DSA_REPORT_GROUP_PROJECT.pdf  # Main 12-page Academic Report (Times New Roman, Pure Black)
│   ├── DSA_REPORT_GROUP_PROJECT.html # HTML version of Report
│   ├── DSA_ALGORITHMS_GUIDE.md       # Comprehensive DSA Reference Guide
│   ├── MADAM_CODE_EXPLANATION.md     # Code Explanation Guide
│   └── archive/                      # Historical / Reference Reports
│       ├── DSA REPORT.pdf            # Reference Report
│       ├── DSA_REPORT_FINAL.pdf
│       ├── DSA_REPORT_PRANAV_PATIL.pdf
│       └── DYPTC_DSA_PROJECT_DOCUMENTATION.pdf
│
├── presentation/                     # Viva Defense Slides & Deck
│   ├── SMART_CAMPUS_LOST_AND_FOUND_PRESENTATION.pptx # 16-Slide Official PowerPoint Presentation
│   └── SMART_CAMPUS_LOST_AND_FOUND_PRESENTATION.html # Interactive Browser Slide Deck
│
├── src/                              # C++17 Application Source Code
│   ├── main.cpp                      # Crow HTTP Web Server & Route Handlers
│   ├── dsa_engine.hpp                # DSA Core (Hash Table, B-Tree, Heaps, Graphs, Tries)
│   ├── database.hpp                  # SQLite3 Manager, Indexes & WAL mode
│   ├── models.hpp                    # OOP Entities (Item, Claim, Location, AuditEntry)
│   └── html_renderer.hpp             # Template Loader (checks templates/ & static/)
│
├── templates/                        # Frontend Web Templates (HTML5)
│   ├── index.html                    # Main Portal & Discovery Feed
│   ├── lost.html                     # Report Lost Item Form
│   ├── found.html                    # Report Found Item & Security Question Setup
│   ├── admin.html                    # Administrative Panel
│   ├── analytics.html                # Campus Recovery Metrics & Trends
│   ├── heatmap.html                  # Location Hotspots Visualizer
│   ├── details.html                  # Deep Item Details
│   ├── track.html                    # Claim Status Tracking
│   ├── matches.html                  # Candidate Matches
│   ├── receipt.html                  # Handover Passcode Slip
│   ├── honors.html                   # Honesty Leaderboard
│   ├── items.html                    # Public Catalog
│   ├── login.html                    # Admin Login
│   ├── search.html                   # Search Interface
│   └── style.css                     # Stylesheet fallback
│
├── static/                           # Static Assets
│   └── style.css                     # Primary Responsive Stylesheet
│
├── data/                             # Persistence & Uploads
│   ├── lost_found.db                 # SQLite3 Database
│   └── uploads/                      # Diagram PNGs, SVGs & Item Photos
│
├── api/                              # Cloud / Serverless Function (Vercel)
│   └── index.js
│
├── include/                          # Third-Party C++ Headers
│   └── crow_all.h                    # Crow C++ Micro-Framework
│
├── smart_campus.exe                  # Precompiled Windows Native Binary
├── Dockerfile                        # Docker Container Config
├── package.json                      # Vercel Deployment Config
├── render.yaml                       # Render Cloud Config
├── vercel.json                       # Vercel Rewrites
└── README.md                         # Project Guide
```
