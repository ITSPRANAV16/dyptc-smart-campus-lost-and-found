#pragma once

#include "models.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include <regex>

namespace html {

// HTML Entity Escaper to prevent XSS attacks
inline std::string escape(const std::string& data) {
 std::string buffer;
 buffer.reserve(data.size());
 for (size_t pos = 0; pos != data.size(); ++pos) {
 switch (data[pos]) {
 case '&': buffer.append("&amp;"); break;
 case '\"': buffer.append("&quot;"); break;
 case '\'': buffer.append("&#39;"); break;
 case '<': buffer.append("&lt;"); break;
 case '>': buffer.append("&gt;"); break;
 default: buffer.append(&data[pos], 1); break;
 }
 }
 return buffer;
}

// URL Decoder for form submission values
inline std::string urlDecode(const std::string& in) {
 std::string out;
 out.reserve(in.size());
 for (size_t i = 0; i < in.size(); ++i) {
 if (in[i] == '%') {
 if (i + 2 < in.size()) {
 int hexVal = 0;
 std::istringstream iss(in.substr(i + 1, 2));
 if (iss >> std::hex >> hexVal) {
 out += static_cast<char>(hexVal);
 i += 2;
 } else {
 out += in[i];
 }
 }
 } else if (in[i] == '+') {
 out += ' ';
 } else {
 out += in[i];
 }
 }
 return out;
}

// Parses application/x-www-form-urlencoded POST body
inline std::unordered_map<std::string, std::string> parseUrlEncoded(const std::string& body) {
 std::unordered_map<std::string, std::string> params;
 std::istringstream stream(body);
 std::string pair;

 while (std::getline(stream, pair, '&')) {
 size_t eqPos = pair.find('=');
 if (eqPos != std::string::npos) {
 std::string key = urlDecode(pair.substr(0, eqPos));
 std::string val = urlDecode(pair.substr(eqPos + 1));
 params[key] = val;
 } else if (!pair.empty()) {
 params[urlDecode(pair)] = "";
 }
 }
 return params;
}

// Contact Masking: Protects student privacy on public pages
inline std::string maskContactInfo(const std::string& contact, bool isAdmin) {
 if (isAdmin) return escape(contact);
 if (contact.empty()) return "<em>Not provided</em>";

 // If it's an email like aryan.sharma@dypatiltcs.com -> a***@dypatiltcs.com
 size_t atPos = contact.find('@');
 if (atPos != std::string::npos && atPos > 1) {
 std::string namePart = contact.substr(0, atPos);
 std::string domainPart = contact.substr(atPos);
 return escape(namePart.substr(0, 2) + "***" + domainPart);
 }

 // If it has phone numbers or mixed text
 if (contact.length() > 6) {
 return escape(contact.substr(0, 4) + "****" + contact.substr(contact.length() - 2));
 }
 return "<em>Contact Protected</em>";
}

// Reads file content from disk (supports root, templates/, and static/ folders)
inline std::string readFile(const std::string& filePath) {
  std::ifstream file(filePath, std::ios::binary);
  if (file.is_open()) {
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }
  std::ifstream file2("templates/" + filePath, std::ios::binary);
  if (file2.is_open()) {
    std::stringstream buffer;
    buffer << file2.rdbuf();
    return buffer.str();
  }
  std::ifstream file3("static/" + filePath, std::ios::binary);
  if (file3.is_open()) {
    std::stringstream buffer;
    buffer << file3.rdbuf();
    return buffer.str();
  }
  return "";
}

// Replaces all occurrences of placeholder in template
inline void replacePlaceholder(std::string& str, const std::string& from, const std::string& to) {
 if (from.empty()) return;
 size_t start_pos = 0;
 while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
 str.replace(start_pos, from.length(), to);
 start_pos += to.length();
 }
}

// Status Pill HTML generator
inline std::string renderStatusPill(const std::string& status) {
 std::string s = status;
 std::transform(s.begin(), s.end(), s.begin(), ::toupper);
 std::string cssClass = "status-pill ";

 if (s == "OPEN") cssClass += "status-open";
 else if (s == "IN_REVIEW") cssClass += "status-review";
 else if (s == "MATCHED") cssClass += "status-matched";
 else if (s == "RETURNED" || s == "RESOLVED") cssClass += "status-returned";
 else cssClass += "status-closed";

 return "<span class=\"" + cssClass + "\">" + escape(s) + "</span>";
}

// Type Pill HTML generator
inline std::string renderTypePill(const std::string& type) {
 std::string t = type;
 std::transform(t.begin(), t.end(), t.begin(), ::toupper);
 if (t == "LOST") {
 return "<span class=\"type-pill type-lost\">LOST</span>";
 } else {
 return "<span class=\"type-pill type-found\">FOUND</span>";
 }
}

// Category Badge
inline std::string renderCategoryBadge(const std::string& category) {
 return "<span class=\"category-tag\">" + escape(category) + "</span>";
}

// Score Bar HTML generator
inline std::string renderScoreBar(double score) {
 int s = static_cast<int>(std::round(score));
 std::string colorClass = "score-low";
 if (s >= 75) colorClass = "score-high";
 else if (s >= 50) colorClass = "score-mid";

 std::ostringstream oss;
 oss << "<div class=\"score-container\">"
 << " <div class=\"score-label\">"
 << " <strong>" << s << "% Match</strong>"
 << " </div>"
 << " <div class=\"score-bar-bg\">"
 << " <div class=\"score-bar-fill " << colorClass << "\" style=\"width: " << s << "%;\"></div>"
 << " </div>"
 << "</div>";
 return oss.str();
}

// Bilingual English / Marathi Translation Engine (Feature 10)
inline std::string tr(const std::string& key, const std::string& lang) {
    if (lang != "mr") return key;
    static const std::unordered_map<std::string, std::string> dict = {
        {"Dashboard", "डॅशबोर्ड"},
        {"Browse", "कॅम्पस वस्तू"},
        {"Track Status", "स्थिती तपासा"},
        {"Search", "शोध"},
        {"Matches", "जुळण्या"},
        {"Analytics", "अल्गोरिदम"},
        {"Honors", "सन्मान फलक"},
        {"Heatmap", "परिसर नकाशा"},
        {"Report Lost", "हरवलेली नोंदवा"},
        {"Report Found", "सापडलेली नोंदवा"},
        {"Admin", "प्रशासक"},
        {"Lost", "हरवलेली"},
        {"Found", "सापडलेली"},
        {"Print Handover Receipt", "हस्तांतरण पावती छापा"},
        {"Verify Ownership", "मालकी पडताळणी"}
    };
    auto it = dict.find(key);
    if (it != dict.end()) return it->second;
    return key;
}

// Navigation Bar with Floating Glass Island Design (Mobile Responsive + Bilingual)
inline std::string renderNavbar(const std::string& activePage, const std::string& lang = "en", bool isAdmin = false) {
    auto isActive = [&](const std::string& page) {
        return (activePage == page) ? "active" : "";
    };

    std::ostringstream oss;
    oss << R"(<!-- Student Campus Hub Top Bar -->
<div class="univ-topbar">
  <div class="univ-topbar-container">
    <div class="univ-badge">
      <span class="univ-seal"><img src="/data/uploads/dyp_logo.png" alt="DYPTC Logo" style="height: 22px; width: auto; vertical-align: middle; filter: drop-shadow(0 1px 2px rgba(0,0,0,0.3));"></span>
      <span><strong>Dr. D. Y. Patil Technical Campus</strong> &bull; Talegaon-Varale, Pune &bull; <small style="color: #fbbf24; font-weight: 700;">DTE Code: 6991</small></span>
    </div>
    <div class="univ-notice-ticker" style="display: flex; align-items: center; gap: 0.85rem;">
      <span><span class="status-dot"></span> <strong>SPPU Affiliated</strong> &bull; C++17 DSA Portal</span>
      <div class="lang-switch-pill" style="display: inline-flex; align-items: center; background: rgba(255,255,255,0.18); padding: 2px 10px; border-radius: 9999px; font-size: 0.76rem; border: 1px solid rgba(255,255,255,0.3);">
        <a href="?lang=en" style="color: )" << (lang == "mr" ? "rgba(255,255,255,0.7)" : "#fbbf24") << R"(; font-weight: 800; text-decoration: none;">EN</a>
        <span style="margin: 0 5px; opacity: 0.4;">|</span>
        <a href="?lang=mr" style="color: )" << (lang == "mr" ? "#fbbf24" : "rgba(255,255,255,0.7)") << R"(; font-weight: 800; text-decoration: none;">मराठी</a>
      </div>
    </div>
  </div>
</div>
<header class="navbar-wrapper">
  <nav class="navbar">
    <div class="nav-container">
      <!-- Brand & Mobile Admin Bar -->
      <div class="nav-brand-bar">
        <a href="/" class="brand-logo">
          <span class="brand-icon" style="background: #ffffff; border: 1.5px solid rgba(220, 38, 38, 0.4); box-shadow: 0 4px 12px rgba(220, 38, 38, 0.15);">
            <img src="/data/uploads/dyp_logo.png" alt="DYPTC Emblem" style="height: 26px; width: auto; object-fit: contain;">
          </span>
          <div class="brand-text-block">
            <span class="brand-text"><strong>DYPTC</strong> Campus</span>
            <span class="brand-subtext">Dr. D. Y. Patil Technical Campus</span>
          </div>
        </a>
        <a href="/admin" class="nav-action-btn nav-action-admin mobile-admin-btn )" << isActive("admin") << R"(" title="Staff &amp; Admin Management">
          <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="11" width="18" height="11" rx="2" ry="2"/><path d="M7 11V7a5 5 0 0 1 10 0v4"/></svg>
          <span>)" << tr("Admin", lang) << R"(</span>
        </a>
      </div>

      <!-- Center Nav Links with Micro-Icons -->
      <ul class="nav-links">
        <li><a href="/" class=")" << isActive("home") << R"("><svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/><polyline points="9 22 9 12 15 12 15 22"/></svg><span>)" << tr("Dashboard", lang) << R"(</span></a></li>
)";
    if (isAdmin) {
        oss << "        <li><a href=\"/items\" class=\"" << isActive("items") << "\" title=\"Master Catalog [Admin Only]\"><svg width=\"15\" height=\"15\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><rect x=\"3\" y=\"3\" width=\"7\" height=\"7\"/><rect x=\"14\" y=\"3\" width=\"7\" height=\"7\"/><rect x=\"14\" y=\"14\" width=\"7\" height=\"7\"/><rect x=\"3\" y=\"14\" width=\"7\" height=\"7\"/></svg><span>" << tr("Browse", lang) << "</span></a></li>\n";
        oss << "        <li><a href=\"/search\" class=\"" << isActive("search") << "\" title=\"DSA Search Catalog [Admin Only]\"><svg width=\"15\" height=\"15\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><circle cx=\"11\" cy=\"11\" r=\"8\"/><line x1=\"21\" y1=\"21\" x2=\"16.65\" y2=\"16.65\"/></svg><span>" << tr("Search", lang) << "</span></a></li>\n";
    } else {
        oss << "        <li><a href=\"/track\" class=\"" << isActive("track") << "\" title=\"Track Status of Your Lost Report\"><svg width=\"15\" height=\"15\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><circle cx=\"11\" cy=\"11\" r=\"8\"/><line x1=\"21\" y1=\"21\" x2=\"16.65\" y2=\"16.65\"/></svg><span>" << tr("Track Status", lang) << "</span></a></li>\n";
    }
    oss << R"(        <li><a href="/matches" class=")" << isActive("matches") << R"("><svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"/></svg><span>)" << tr("Matches", lang) << R"(</span></a></li>
        <li><a href="/analytics" class=")" << isActive("analytics") << R"("><svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M18 20V10M12 20V4M6 20v-6"/></svg><span>)" << tr("Analytics", lang) << R"(</span></a></li>
        <li><a href="/honors" class=")" << isActive("honors") << R"("><svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="8" r="7"/><polyline points="8.21 13.89 7 23 12 20 17 23 15.79 13.88"/></svg><span>)" << tr("Honors", lang) << R"(</span></a></li>
        <li><a href="/heatmap" class=")" << isActive("heatmap") << R"("><svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="1" y="4" width="22" height="16" rx="2"/><line x1="1" y1="10" x2="23" y2="10"/><line x1="8" y1="4" x2="8" y2="20"/><line x1="16" y1="4" x2="16" y2="20"/></svg><span>)" << tr("Heatmap", lang) << R"(</span></a></li>
      </ul>

      <!-- Right Action CTA Buttons -->
      <div class="nav-actions">
        <a href="/lost" class="nav-action-btn nav-action-lost )" << isActive("lost") << R"("><svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.4" stroke-linecap="round" stroke-linejoin="round"><line x1="12" y1="5" x2="12" y2="19"/><line x1="5" y1="12" x2="19" y2="12"/></svg><span>)" << tr("Report Lost", lang) << R"(</span></a>
)";
    if (isAdmin) {
        oss << "        <a href=\"/found\" class=\"nav-action-btn nav-action-found " << isActive("found") << "\"><svg width=\"14\" height=\"14\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2.4\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><polyline points=\"20 6 9 17 4 12\"/></svg><span>" << tr("Report Found", lang) << "</span></a>\n";
    } else {
        oss << "        <a href=\"/track\" class=\"nav-action-btn nav-action-found " << isActive("track") << "\" style=\"background: rgba(37,99,235,0.08); color: #2563eb; border-color: rgba(37,99,235,0.25);\"><svg width=\"14\" height=\"14\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2.4\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><circle cx=\"11\" cy=\"11\" r=\"8\"/><line x1=\"21\" y1=\"21\" x2=\"16.65\" y2=\"16.65\"/></svg><span>" << tr("Track Status", lang) << "</span></a>\n";
    }
    oss << R"(        <a href="/admin" class="nav-action-btn nav-action-admin desktop-admin-btn )" << isActive("admin") << R"(" title="Staff &amp; Admin Management"><svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="11" width="18" height="11" rx="2" ry="2"/><path d="M7 11V7a5 5 0 0 1 10 0v4"/></svg><span>)" << tr("Admin", lang) << R"(</span></a>
      </div>
    </div>
  </nav>
</header>)";
    return oss.str();
}

// Claim Token Badge HTML generator (Feature 07)
inline std::string renderClaimTokenBadge(const std::string& token) {
    if (token.empty()) return "";
    return "<div class=\"claim-token-badge\" title=\"Official DYPTC Verification Security Token\">"
           "<span class=\"claim-token-label\">CLAIM TOKEN</span>"
           "<strong class=\"claim-token-val\">" + escape(token) + "</strong>"
           "</div>";
}

// Institutional RFC 4180 CSV Generator (Feature 13)
inline std::string generateInstitutionalCSV(const std::vector<Item>& items) {
    std::ostringstream csv;
    csv << "Item ID,Type,Claim Token,Item Name,Category,Colour,Location,Report Date,Department,Reporter Name,Student ID,Status,Created At\r\n";
    for (const auto& item : items) {
        csv << item.id << ","
            << "\"" << escape(item.type) << "\","
            << "\"" << escape(item.claim_token) << "\","
            << "\"" << escape(item.name) << "\","
            << "\"" << escape(item.category) << "\","
            << "\"" << escape(item.colour) << "\","
            << "\"" << escape(item.location) << "\","
            << "\"" << escape(item.date) << "\","
            << "\"" << escape(item.department) << "\","
            << "\"" << escape(item.contact_name) << "\","
            << "\"" << escape(item.student_id) << "\","
            << "\"" << escape(item.status) << "\","
            << "\"" << escape(item.created_at) << "\"\r\n";
    }
    return csv.str();
}

// Audit Log Timeline HTML generator (Feature 12)
inline std::string renderAuditTimeline(const std::vector<AuditLogEntry>& entries) {
    if (entries.empty()) {
        return "<p style=\"color: #64748b; font-size: 0.88rem;\">No security audit logs recorded yet.</p>";
    }
    std::ostringstream oss;
    oss << "<div class=\"audit-timeline\">";
    for (const auto& e : entries) {
        std::string badgeColor = (e.action == "REPORTED" ? "#2563eb" : (e.action == "STATUS_CHANGE" ? "#d97706" : "#059669"));
        oss << "<div class=\"audit-timeline-item\">"
            << "  <div class=\"audit-dot\" style=\"background: " << badgeColor << ";\"></div>"
            << "  <div class=\"audit-content\">"
            << "    <div class=\"audit-header\">"
            << "      <span class=\"audit-action\" style=\"color: " << badgeColor << "; font-weight: 700;\">" << escape(e.action) << "</span>"
            << "      <span class=\"audit-time\">" << escape(e.timestamp) << "</span>"
            << "    </div>"
            << "    <div class=\"audit-detail\">"
            << "      <span>Transition: <code>" << (e.old_status.empty() ? "START" : escape(e.old_status)) << " &rarr; " << escape(e.new_status) << "</code></span>"
            << "      &bull; <span>Officer/ID: <strong>" << escape(e.performed_by) << "</strong></span>"
            << "    </div>"
            << "    <div class=\"audit-remarks\">" << escape(e.remarks) << "</div>"
            << "  </div>"
            << "</div>";
    }
    oss << "</div>";
    return oss.str();
}


// Footer
inline std::string renderFooter() {
    return R"(
    <footer class="footer">
    <div class="footer-container">
    <div class="footer-col">
    <h4>Dr. D. Y. Patil Technical Campus</h4>
    <p><strong>D Y Patil Academic Education Excellence Federation's (DYPAEEF)</strong><br>
    Approved by AICTE New Delhi &bull; Recognized by DTE Govt. of Maharashtra (DTE Code: <strong>6991</strong>) &bull; Affiliated to <strong>Savitribai Phule Pune University (SPPU)</strong>.</p>
    </div>
    <div class="footer-col">
    <h4>Academic Wings &amp; Facilities</h4>
    <p>Engineering (CE, CS, AI&amp;DS, ECE, FE), MCA, and Architecture. Campus features Central High-Tech Library, Innovation Labs, Auditorium, and Sports Complex.</p>
    </div>
    <div class="footer-col">
    <h4>Campus Security &amp; Care Desk</h4>
    <p>Central Security Office &bull; Admin Building Ground Floor<br>
    Sr. No. 124 &amp; 126, A/P Varale, Talegaon Dabhade, Pune &ndash; 410507<br>
    Email: <strong>contact@dypatiltcs.com</strong><br>
    Helpline: <strong>(+91) 7666829653 &bull; (+91) 7822099218</strong></p>
    </div>
    </div>
    <div class="footer-bottom">
    <p>&copy; 2026 Dr. D. Y. Patil Technical Campus (DYPTC), Talegaon, Pune. Pure HTML5 &amp; CSS3 &bull; C++17 DSA Algorithmic Engine.</p>
    </div>
    </footer>
    )";
}

} // namespace html
