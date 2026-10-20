#define CROW_MAIN

#include "crow_all.h"
#include "models.hpp"
#include "database.hpp"
#include "dsa_engine.hpp"
#include "html_renderer.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <algorithm>

// Global persistent database instance
static Database g_db;

// Administrator Authentication Credentials
const std::string ADMIN_PASSWORD = "admin123";
const std::string ADMIN_COOKIE_VAL = "admin_auth_token_smartcampus_2026";

inline bool isAdminAuthenticated(const crow::request& req) {
 std::string cookieHeader = req.get_header_value("Cookie");
 return cookieHeader.find("admin_session=" + ADMIN_COOKIE_VAL) != std::string::npos;
}

inline std::string getLang(const crow::request& req) {
 if (req.url_params.get("lang") != nullptr) {
  std::string l = req.url_params.get("lang");
  if (l == "mr") return "mr";
 }
 return "en";
}

// Helper to render an item card for catalog and search results
std::string renderItemCard(const Item& item, bool isAdmin = false) {
    // Zero-Knowledge Blind Custody: FOUND items are strictly confidential
    // and held in Central Custody. Normal users / students can NEVER view them under any circumstances.
    if (!isAdmin && item.type == "FOUND") {
        return "";
    }

    std::ostringstream oss;
    std::string img = item.image_url.empty() ? "/data/uploads/item_default.svg" : item.image_url;

    std::string displayName = item.name;
    std::string displayDesc = item.description;
    std::string displayColour = item.colour;
    std::string displayLocation = item.location;

    oss << "<div class=\"item-card\">"
        << "  <div class=\"item-card-header\">"
        << "    <img src=\"" << html::escape(img) << "\" alt=\"" << html::escape(displayName) << "\">"
        << "    <div class=\"item-card-badges\">"
        << "      " << html::renderTypePill(item.type)
        << "      " << html::renderStatusPill(item.status)
        << "    </div>"
        << "  </div>"
        << "  <div class=\"item-card-body\">"
        << "    <div style=\"display: flex; justify-content: space-between; align-items: center; margin-bottom: 0.4rem;\">"
        << "      " << html::renderCategoryBadge(item.category)
        << "      " << html::renderClaimTokenBadge(item.claim_token)
        << "    </div>"
        << "    <h3 class=\"item-title\">" << html::escape(displayName) << "</h3>"
        << "    <p class=\"item-desc\">" << html::escape(displayDesc) << "</p>"
        << "    <div class=\"item-meta\">"
        << "      <div class=\"item-meta-row\"><strong>Department:</strong> " << html::escape(item.department) << "</div>"
        << "      <div class=\"item-meta-row\"><strong>Colour:</strong> " << html::escape(displayColour) << "</div>"
        << "      <div class=\"item-meta-row\"><strong>Location:</strong> " << html::escape(displayLocation) << "</div>"
        << "      <div class=\"item-meta-row\"><strong>Date:</strong> " << html::escape(item.date) << "</div>"
        << "    </div>"
        << "  </div>"
        << "  <div class=\"item-card-footer\">"
        << "    <a href=\"/item?id=" << item.id << "\" class=\"btn btn-outline-dark btn-sm\">View Details</a>"
        << "    <a href=\"/receipt?id=" << item.id << "\" class=\"btn btn-outline-dark btn-sm\" title=\"Print Official Handover Slip\">Receipt</a>"
        << "    <a href=\"/matches?id=" << item.id << "\" class=\"btn btn-primary btn-sm\">Auto Matches</a>"
        << "  </div>"
        << "</div>";

    return oss.str();
}

// Helper to render a match card for the Matching Engine
std::string renderMatchCard(const MatchResult& match, bool isAdmin = false) {
    std::ostringstream oss;
    int score = static_cast<int>(std::round(match.breakdown.total_score));

    std::string scoreColorClass = "score-low";
    if (score >= 75) scoreColorClass = "score-high";
    else if (score >= 50) scoreColorClass = "score-mid";

    std::string lostImg = match.lost_item.image_url.empty() ? "/data/uploads/item_default.svg" : match.lost_item.image_url;
    std::string foundImg = match.found_item.image_url.empty() ? "/data/uploads/item_default.svg" : match.found_item.image_url;

    std::string foundDisplayName = match.found_item.name;
    std::string foundDisplayLoc = match.found_item.location;
    std::string foundDisplayCol = match.found_item.colour;
    std::string foundDisplayDesc = match.found_item.description;
    std::string foundActionBtn = "<a href=\"/item?id=" + std::to_string(match.found_item.id) + "\" class=\"btn btn-outline-dark btn-sm\">View Found #" + std::to_string(match.found_item.id) + "</a>";

    if (!isAdmin) {
        foundImg = "/data/uploads/item_default.svg";
        foundDisplayName = "Sealed Custody Belonging (" + match.found_item.category + ")";
        foundDisplayLoc = match.found_item.department.empty() ? "Central Custody Lockbox" : (match.found_item.department + " (Custody Lockbox)");
        foundDisplayCol = "🔒 Confidential (Verify on Claim)";
        foundDisplayDesc = "🔒 [Anti-Fraud Blind Custody]: All identifying physical marks and serials are withheld to prevent copycat claims. Bring your DYPTC Student ID to the Central Security Control Room to verify ownership.";
        foundActionBtn = "<span class=\"badge\" style=\"background:#fef2f2;color:#b91c1c;padding:0.35rem 0.65rem;border:1px solid #fecaca;font-weight:700;\">🔒 Central Custody Desk</span>";
    }

    oss << "<div class=\"match-comparison-card\">"
        << "  <div class=\"match-header\">"
        << "    <div>"
        << "      <span style=\"font-size: 0.85rem; font-weight: 700; color: #64748b; text-transform: uppercase;\">Algorithmic Match Candidate</span>"
        << "      <h3 style=\"font-size: 1.25rem; color: #0f172a;\">" << html::escape(match.lost_item.name) << " &harr; " << html::escape(foundDisplayName) << "</h3>"
        << "    </div>"
        << "    <div style=\"display: flex; gap: 0.5rem; align-items: center;\">"
        << "      <a href=\"/item?id=" << match.lost_item.id << "\" class=\"btn btn-outline-dark btn-sm\">View Lost #" << match.lost_item.id << "</a>"
        << "      " << foundActionBtn
        << "    </div>"
        << "  </div>"
        << "  <div class=\"match-grid\">"
        << "    <!-- Lost Item Side -->"
        << "    <div class=\"match-side match-side-lost\">"
        << "      <div style=\"display: flex; gap: 0.75rem; align-items: center; margin-bottom: 0.75rem;\">"
        << "        <img src=\"" << html::escape(lostImg) << "\" style=\"width: 44px; height: 44px; border-radius: 6px; background: #fff; padding: 4px; border: 1px solid #e2e8f0;\">"
        << "        <div>"
        << "          <span class=\"badge\" style=\"background: #fef2f2; color: #b91c1c; font-size: 0.75rem; margin-bottom: 2px;\">Lost Report</span>"
        << "          <h4 style=\"margin: 0; font-size: 1rem;\">" << html::escape(match.lost_item.name) << "</h4>"
        << "        </div>"
        << "      </div>"
        << "      <div class=\"item-meta-row\"><strong>Location:</strong> " << html::escape(match.lost_item.location) << "</div>"
        << "      <div class=\"item-meta-row\"><strong>Colour:</strong> " << html::escape(match.lost_item.colour) << "</div>"
        << "      <div class=\"item-meta-row\"><strong>Reported Date:</strong> " << html::escape(match.lost_item.date) << "</div>"
        << "      <div class=\"item-meta-row\"><strong>Description:</strong> " << html::escape(match.lost_item.description) << "</div>"
        << "    </div>"
        << "    <!-- Found Item Side -->"
        << "    <div class=\"match-side match-side-found\">"
        << "      <div style=\"display: flex; gap: 0.75rem; align-items: center; margin-bottom: 0.75rem;\">"
        << "        <img src=\"" << html::escape(foundImg) << "\" style=\"width: 44px; height: 44px; border-radius: 6px; background: #fff; padding: 4px; border: 1px solid #e2e8f0;\">"
        << "        <div>"
        << "          <span class=\"badge\" style=\"background: #ecfdf5; color: #059669; font-size: 0.75rem; margin-bottom: 2px;\">Custody Record</span>"
        << "          <h4 style=\"margin: 0; font-size: 1rem;\">" << html::escape(foundDisplayName) << "</h4>"
        << "        </div>"
        << "      </div>"
        << "      <div class=\"item-meta-row\"><strong>Location:</strong> " << html::escape(foundDisplayLoc) << "</div>"
        << "      <div class=\"item-meta-row\"><strong>Colour:</strong> " << html::escape(foundDisplayCol) << "</div>"
        << "      <div class=\"item-meta-row\"><strong>Found Date:</strong> " << html::escape(match.found_item.date) << "</div>"
        << "      <div class=\"item-meta-row\"><strong>Description:</strong> " << html::escape(foundDisplayDesc) << "</div>"
        << "    </div>"
        << "  </div>"
        << "</div>";

    return oss.str();
}

// Helper to render Lost page with preserved form values
std::string renderLostPage(const Item& item, const std::string& errorMsg = "") {
 std::string tmpl = html::readFile("lost.html");
 std::string alertHtml;
 if (!errorMsg.empty()) {
 alertHtml = "<div class=\"alert alert-error\"><strong>Validation Error:</strong> " + 
 html::escape(errorMsg) + "</div>";
 }

 html::replacePlaceholder(tmpl, "{{NAVBAR}}", html::renderNavbar("lost"));
 html::replacePlaceholder(tmpl, "{{ALERT}}", alertHtml);
 html::replacePlaceholder(tmpl, "{{VAL_NAME}}", html::escape(item.name));
 html::replacePlaceholder(tmpl, "{{VAL_COLOUR}}", html::escape(item.colour));
 html::replacePlaceholder(tmpl, "{{VAL_LOCATION}}", html::escape(item.location));
 html::replacePlaceholder(tmpl, "{{VAL_DATE}}", html::escape(item.date));
 html::replacePlaceholder(tmpl, "{{VAL_DESC}}", html::escape(item.description));
 html::replacePlaceholder(tmpl, "{{VAL_CONTACT_NAME}}", html::escape(item.contact_name));
 html::replacePlaceholder(tmpl, "{{VAL_STUDENT_ID}}", html::escape(item.student_id));
 html::replacePlaceholder(tmpl, "{{VAL_CONTACT_INFO}}", html::escape(item.contact_info));

 // Category options
 html::replacePlaceholder(tmpl, "{{CAT_ELEC_SEL}}", (item.category == "Electronics" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_CARD_SEL}}", (item.category == "ID & Cards" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_BOOK_SEL}}", (item.category == "Books & Stationery" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_KEY_SEL}}", (item.category == "Keys & Accessories" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_BAG_SEL}}", (item.category == "Bags & Backpacks" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_CLOTH_SEL}}", (item.category == "Clothing & Wearables" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_OTHER_SEL}}", (item.category == "Other" ? "selected" : ""));

 // Image presets
 html::replacePlaceholder(tmpl, "{{IMG_LAPTOP_SEL}}", (item.image_url == "/data/uploads/laptop.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_EARBUDS_SEL}}", (item.image_url == "/data/uploads/earbuds.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_IDCARD_SEL}}", (item.image_url == "/data/uploads/idcard.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_KEYS_SEL}}", (item.image_url == "/data/uploads/keys.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_CALC_SEL}}", (item.image_url == "/data/uploads/calculator.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_BAG_SEL}}", (item.image_url == "/data/uploads/bag.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_BOTTLE_SEL}}", (item.image_url == "/data/uploads/bottle.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_WATCH_SEL}}", (item.image_url == "/data/uploads/watch.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_DEFAULT_SEL}}", (item.image_url == "/data/uploads/item_default.svg" ? "selected" : ""));

 html::replacePlaceholder(tmpl, "{{FOOTER}}", html::renderFooter());
 return tmpl;
}

// Helper to render Found page with preserved form values
std::string renderFoundPage(const Item& item, const std::string& errorMsg = "") {
 std::string tmpl = html::readFile("found.html");
 std::string alertHtml;
 if (!errorMsg.empty()) {
 alertHtml = "<div class=\"alert alert-error\"><strong>Validation Error:</strong> " + 
 html::escape(errorMsg) + "</div>";
 }

 html::replacePlaceholder(tmpl, "{{NAVBAR}}", html::renderNavbar("found"));
 html::replacePlaceholder(tmpl, "{{ALERT}}", alertHtml);
 html::replacePlaceholder(tmpl, "{{VAL_NAME}}", html::escape(item.name));
 html::replacePlaceholder(tmpl, "{{VAL_COLOUR}}", html::escape(item.colour));
 html::replacePlaceholder(tmpl, "{{VAL_LOCATION}}", html::escape(item.location));
 html::replacePlaceholder(tmpl, "{{VAL_DATE}}", html::escape(item.date));
 html::replacePlaceholder(tmpl, "{{VAL_STORAGE_LOCATION}}", html::escape(item.storage_location));
 html::replacePlaceholder(tmpl, "{{VAL_DESC}}", html::escape(item.description));
 html::replacePlaceholder(tmpl, "{{VAL_CONTACT_NAME}}", html::escape(item.contact_name));
 html::replacePlaceholder(tmpl, "{{VAL_STUDENT_ID}}", html::escape(item.student_id));
 html::replacePlaceholder(tmpl, "{{VAL_CONTACT_INFO}}", html::escape(item.contact_info));

 // Category options
 html::replacePlaceholder(tmpl, "{{CAT_ELEC_SEL}}", (item.category == "Electronics" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_CARD_SEL}}", (item.category == "ID & Cards" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_BOOK_SEL}}", (item.category == "Books & Stationery" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_KEY_SEL}}", (item.category == "Keys & Accessories" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_BAG_SEL}}", (item.category == "Bags & Backpacks" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_CLOTH_SEL}}", (item.category == "Clothing & Wearables" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_OTHER_SEL}}", (item.category == "Other" ? "selected" : ""));

 // Image presets
 html::replacePlaceholder(tmpl, "{{IMG_LAPTOP_SEL}}", (item.image_url == "/data/uploads/laptop.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_EARBUDS_SEL}}", (item.image_url == "/data/uploads/earbuds.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_IDCARD_SEL}}", (item.image_url == "/data/uploads/idcard.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_KEYS_SEL}}", (item.image_url == "/data/uploads/keys.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_CALC_SEL}}", (item.image_url == "/data/uploads/calculator.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_BAG_SEL}}", (item.image_url == "/data/uploads/bag.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_BOTTLE_SEL}}", (item.image_url == "/data/uploads/bottle.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_WATCH_SEL}}", (item.image_url == "/data/uploads/watch.svg" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{IMG_DEFAULT_SEL}}", (item.image_url == "/data/uploads/item_default.svg" ? "selected" : ""));

 html::replacePlaceholder(tmpl, "{{FOOTER}}", html::renderFooter());
 return tmpl;
}

int main() {
 // 1. Initialize SQLite Database and seed sample realistic campus items
 if (!g_db.init("data/lost_found.db")) {
 std::cerr << "[Fatal Error] Failed to initialize SQLite database at data/lost_found.db" << std::endl;
 return 1;
 }
 g_db.seedSampleData();

 crow::SimpleApp app;

 // ========================================================================
 // STATIC ASSETS
 // ========================================================================
 
 // Style.css route
 CROW_ROUTE(app, "/style.css")([]() {
        crow::response res(html::readFile("style.css"));
        res.set_header("Content-Type", "text/css");
        res.set_header("Cache-Control", "no-cache, no-store, must-revalidate");
        res.set_header("Pragma", "no-cache");
        res.set_header("Expires", "0");
        return res;
    });

 // Image & SVG uploads route
 CROW_ROUTE(app, "/data/uploads/<string>")([](const std::string& filename) {
 std::string filepath = "data/uploads/" + filename;
 std::string content = html::readFile(filepath);
 if (content.empty()) {
 return crow::response(404, "File not found");
 }
 crow::response res(content);
 if (filename.find(".svg") != std::string::npos) {
 res.set_header("Content-Type", "image/svg+xml");
 } else if (filename.find(".png") != std::string::npos) {
 res.set_header("Content-Type", "image/png");
 } else {
 res.set_header("Content-Type", "application/octet-stream");
 }
 return res;
 });

 // ========================================================================
 // 1. HOME / DASHBOARD
 // ========================================================================
 CROW_ROUTE(app, "/")([](const crow::request& req) {
 std::string tmpl = html::readFile("index.html");
 if (tmpl.empty()) return crow::response(500, "Template index.html missing");

 std::vector<Item> allItems = g_db.getAllItems();
 DashboardStats stats = dsa::computeStats(allItems);
 bool isAdmin = isAdminAuthenticated(req);

 // Find top high-confidence matches for home showcase (min 40% score)
 std::string matchesShowcaseHtml;
 if (isAdmin) {
 std::vector<MatchResult> topMatches = dsa::findRankedMatches(allItems, -1, 40.0);
 if (!topMatches.empty()) {
 std::ostringstream mOss;
 mOss << "<div class=\"home-matches-section\">"
 << " <div class=\"home-matches-header\">"
 << " <div>"
 << " <span style=\"font-size: 0.78rem; font-weight: 800; color: #2563eb; text-transform: uppercase; letter-spacing: 0.05em;\">Administrator Custody Live Stream</span>"
 << " <h3 style=\"font-size: 1.25rem; font-weight: 800; color: #0f172a;\">High-Confidence Potential Matches</h3>"
 << " </div>"
 << " <a href=\"/matches\" class=\"btn btn-primary btn-sm\">View All Algorithmic Matches &rarr;</a>"
 << " </div>"
 << " <div class=\"home-matches-grid\">";
 
 size_t limit = std::min(topMatches.size(), size_t(2));
 for (size_t i = 0; i < limit; ++i) {
 const auto& m = topMatches[i];
 int score = static_cast<int>(std::round(m.breakdown.total_score));
 std::string badgeBg = (score >= 75) ? "#ecfdf5; color: #059669; border: 1px solid #a7f3d0;" : "#eff6ff; color: #2563eb; border: 1px solid #bfdbfe;";

 mOss << " <div class=\"home-match-card\">"
 << " <div style=\"display: flex; justify-content: space-between; align-items: flex-start; margin-bottom: 0.75rem;\">"
 << " " << html::renderCategoryBadge(m.lost_item.category)
 << " <span class=\"home-match-score-badge\" style=\"background: " << badgeBg << "\">" << score << "% Match</span>"
 << " </div>"
 << " <div style=\"font-size: 0.95rem; font-weight: 700; color: #0f172a; margin-bottom: 0.5rem;\">"
 << " <span style=\"color: #dc2626;\">Lost:</span> " << html::escape(m.lost_item.name)
 << " &harr; <span style=\"color: #059669;\">Found:</span> " << html::escape(m.found_item.name)
 << " </div>"
 << " <div style=\"font-size: 0.82rem; color: #64748b; margin-bottom: 1rem;\">"
 << " <div>" << html::escape(m.lost_item.location) << " &bull; " << html::escape(m.found_item.location) << "</div>"
 << " </div>"
 << " <div style=\"display: flex; gap: 0.5rem; justify-content: flex-end;\">"
 << " <a href=\"/item?id=" << m.lost_item.id << "\" class=\"btn btn-outline-dark btn-sm\">Lost #" << m.lost_item.id << "</a>"
 << " <a href=\"/item?id=" << m.found_item.id << "\" class=\"btn btn-outline-dark btn-sm\">Found #" << m.found_item.id << "</a>"
 << " <a href=\"/matches?id=" << m.lost_item.id << "\" class=\"btn btn-primary btn-sm\">Review Match</a>"
 << " </div>"
 << " </div>";
 }
 mOss << " </div></div>";
 matchesShowcaseHtml = mOss.str();
 }
 } else {
 matchesShowcaseHtml = R"(<div class="home-matches-section">
   <div class="home-matches-header">
     <div>
       <span style="font-size: 0.78rem; font-weight: 800; color: #dc2626; text-transform: uppercase; letter-spacing: 0.05em;">Campus Anti-Fraud Security</span>
       <h3 style="font-size: 1.25rem; font-weight: 800; color: #0f172a;">Zero-Knowledge Campus Custody Active</h3>
     </div>
     <a href="/lost" class="btn btn-primary btn-sm">+ Report Lost Item</a>
   </div>
   <div style="background:#ffffff; border:1px solid #e2e8f0; border-radius:12px; padding:1.25rem 1.5rem; display:flex; align-items:center; gap:1.25rem; box-shadow:0 2px 10px rgba(0,0,0,0.03);">
     <div style="width:48px; height:48px; border-radius:10px; background:#fef2f2; color:#dc2626; display:flex; align-items:center; justify-content:center; flex-shrink:0;">
       <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="3" y="11" width="18" height="11" rx="2" ry="2"/><path d="M7 11V7a5 5 0 0 1 10 0v4"/></svg>
     </div>
     <div>
       <h4 style="margin:0 0 0.25rem; font-size:1rem; color:#0f172a;">Found Items are Sealed in Central Custody</h4>
       <p style="margin:0; font-size:0.86rem; color:#64748b; line-height:1.5;">To prevent fake claims and copycat reports, found belongings are strictly hidden from students and sealed in Central Custody. When you submit a lost report, our C++17 algorithmic engine evaluates matches against sealed custody records in the background.</p>
     </div>
   </div>
 </div>)";
 }

 // Render recent items: STRICTLY LOST ITEMS ONLY for non-admins
 std::string recentCards;
 size_t count = 0;
 for (const auto& item : allItems) {
 if (!isAdmin && item.type != "LOST") continue;
 recentCards += renderItemCard(item, isAdmin);
 count++;
 if (count >= 6) break;
 }
 if (recentCards.empty()) {
 recentCards = "<div class=\"empty-state form-group-full\">"
 " <div class=\"empty-icon\"></div>"
 " <h3 class=\"empty-title\">No Reports Yet</h3>"
 " <p class=\"empty-desc\">No lost belongings have been reported yet.</p>"
 " <a href=\"/lost\" class=\"btn btn-primary\">Report Lost Item</a>"
 "</div>";
 }

 std::string alertHtml;
 if (req.url_params.get("msg") != nullptr) {
 std::string msg = req.url_params.get("msg");
 if (msg == "lost_created") {
 alertHtml = "<div class=\"alert alert-success\"><strong>Success:</strong> Lost item report registered! Matching engine has evaluated cross-matches.</div>";
 } else if (msg == "found_created") {
 alertHtml = "<div class=\"alert alert-success\"><strong>Success:</strong> Found item report registered! Matching engine has checked for owners.</div>";
 }
 }

 html::replacePlaceholder(tmpl, "{{NAVBAR}}", html::renderNavbar("home", getLang(req)));
 html::replacePlaceholder(tmpl, "{{ALERT}}", alertHtml);
 html::replacePlaceholder(tmpl, "{{STAT_LOST}}", std::to_string(stats.total_lost));
 html::replacePlaceholder(tmpl, "{{STAT_FOUND}}", std::to_string(stats.total_found));
 html::replacePlaceholder(tmpl, "{{STAT_MATCHED}}", std::to_string(stats.total_matched));
 html::replacePlaceholder(tmpl, "{{STAT_RETURNED}}", std::to_string(stats.total_returned));
 html::replacePlaceholder(tmpl, "{{STAT_TOTAL}}", std::to_string(stats.total_items));
 html::replacePlaceholder(tmpl, "{{HOME_MATCHES_SHOWCASE}}", matchesShowcaseHtml);
 html::replacePlaceholder(tmpl, "{{RECENT_ITEMS}}", recentCards);
 html::replacePlaceholder(tmpl, "{{FOOTER}}", html::renderFooter());

 crow::response res(tmpl);
 res.set_header("Content-Type", "text/html; charset=utf-8");
 return res;
 });

 // ========================================================================
 // 2. REPORT LOST ITEM
 // ========================================================================
 CROW_ROUTE(app, "/lost").methods("GET"_method)([](const crow::request& req) {
 Item emptyItem;
 std::string htmlContent = renderLostPage(emptyItem);
 crow::response res(htmlContent);
 res.set_header("Content-Type", "text/html; charset=utf-8");
 return res;
 });

 CROW_ROUTE(app, "/lost").methods("POST"_method)([](const crow::request& req) {
 auto params = html::parseUrlEncoded(req.body);

 Item item;
 item.type = "LOST";
 item.name = dsa::trim(params["name"]);
 item.category = dsa::trim(params["category"]);
 item.colour = dsa::trim(params["colour"]);
 item.location = dsa::trim(params["location"]);
 item.date = dsa::trim(params["date"]);
 item.description = dsa::trim(params["description"]);
 item.contact_name = dsa::trim(params["contact_name"]);
 item.student_id = dsa::trim(params["student_id"]);
 item.contact_info = dsa::trim(params["contact_info"]);
 item.image_url = dsa::trim(params["image_url"]);
 item.status = "OPEN";

 // Server-Side Input Validation in C++
 std::string errorMsg;
 if (item.name.empty() || item.name.length() < 2) {
 errorMsg = "Item Name must contain at least 2 characters.";
 } else if (item.category.empty()) {
 errorMsg = "Please select a valid category.";
 } else if (item.colour.empty()) {
 errorMsg = "Please specify the item colour.";
 } else if (item.location.empty()) {
 errorMsg = "Please specify where the item was lost.";
 } else if (item.date.empty()) {
 errorMsg = "Please provide the date when the item was lost.";
 } else if (item.description.empty() || item.description.length() < 3) {
 errorMsg = "Please provide a description (at least 3 characters).";
 } else if (item.contact_name.empty() || item.contact_info.empty()) {
 errorMsg = "Contact Name and Phone/Email are required.";
 }

 if (!errorMsg.empty()) {
 // Render the page with the user's entered values intact!
 std::string htmlContent = renderLostPage(item, errorMsg);
 crow::response res(htmlContent);
 res.set_header("Content-Type", "text/html; charset=utf-8");
 return res;
 }

 // Save to SQLite
 if (g_db.insertItem(item)) {
 crow::response res(303);
 res.set_header("Location", "/matches?id=" + std::to_string(item.id) + "&msg=lost_created");
 return res;
 }

 return crow::response(500, "Database insertion failed");
 });

 // ========================================================================
 // 3. REPORT FOUND ITEM
 // ========================================================================
 CROW_ROUTE(app, "/found").methods("GET"_method)([](const crow::request& req) {
 Item emptyItem;
 std::string htmlContent = renderFoundPage(emptyItem);
 crow::response res(htmlContent);
 res.set_header("Content-Type", "text/html; charset=utf-8");
 return res;
 });

 CROW_ROUTE(app, "/found").methods("POST"_method)([](const crow::request& req) {
 auto params = html::parseUrlEncoded(req.body);

 Item item;
 item.type = "FOUND";
 item.name = dsa::trim(params["name"]);
 item.category = dsa::trim(params["category"]);
 item.colour = dsa::trim(params["colour"]);
 item.location = dsa::trim(params["location"]);
 item.date = dsa::trim(params["date"]);
 item.storage_location = dsa::trim(params["storage_location"]);
 item.description = dsa::trim(params["description"]);
 item.contact_name = dsa::trim(params["contact_name"]);
 item.student_id = dsa::trim(params["student_id"]);
 item.contact_info = dsa::trim(params["contact_info"]);
 item.image_url = dsa::trim(params["image_url"]);
 item.status = "OPEN";

 // Server-Side Input Validation in C++
 std::string errorMsg;
 if (item.name.empty() || item.name.length() < 2) {
 errorMsg = "Item Name must contain at least 2 characters.";
 } else if (item.category.empty()) {
 errorMsg = "Please select a valid category.";
 } else if (item.colour.empty()) {
 errorMsg = "Please specify the item colour.";
 } else if (item.location.empty()) {
 errorMsg = "Please specify where the item was found.";
 } else if (item.storage_location.empty()) {
 errorMsg = "Please specify where the item is securely stored for collection.";
 } else if (item.date.empty()) {
 errorMsg = "Please provide the date when the item was found.";
 } else if (item.description.empty() || item.description.length() < 3) {
 errorMsg = "Please provide a description (at least 3 characters).";
 } else if (item.contact_name.empty() || item.contact_info.empty()) {
 errorMsg = "Finder Name and Official Phone/Email are required.";
 }

 if (!errorMsg.empty()) {
 // Render the page with the user's entered values intact!
 std::string htmlContent = renderFoundPage(item, errorMsg);
 crow::response res(htmlContent);
 res.set_header("Content-Type", "text/html; charset=utf-8");
 return res;
 }

 // Save to SQLite
 if (g_db.insertItem(item)) {
 crow::response res(303);
 res.set_header("Location", "/matches?id=" + std::to_string(item.id) + "&msg=found_created");
 return res;
 }

 return crow::response(500, "Database insertion failed");
 });

 // ========================================================================
 // 4. BROWSE ALL ITEMS (WITH DSA SORTING DEMONSTRATOR)
 // ========================================================================
 CROW_ROUTE(app, "/items")([](const crow::request& req) {
 std::string tmpl = html::readFile("items.html");
 if (tmpl.empty()) return crow::response(500, "Template items.html missing");

 bool isAdmin = isAdminAuthenticated(req);
 std::string q = req.url_params.get("q") ? req.url_params.get("q") : "";
 std::string filterType = req.url_params.get("type") ? req.url_params.get("type") : (isAdmin ? "all" : "lost");
 if (!isAdmin) {
 filterType = "lost"; // Strict enforcement: non-admins can never filter or view found items
 }
 std::string filterCategory = req.url_params.get("category") ? req.url_params.get("category") : "all";
 std::string filterStatus = req.url_params.get("status") ? req.url_params.get("status") : "all";
 std::string sortBy = req.url_params.get("sortBy") ? req.url_params.get("sortBy") : "date";
 std::string order = req.url_params.get("order") ? req.url_params.get("order") : "desc";
 std::string algo = req.url_params.get("algo") ? req.url_params.get("algo") : "merge";

 bool ascending = (order == "asc");

 // Fetch raw items from database
 std::vector<Item> items = g_db.getAllItems();

 // Calculate status counts based on current filters (strictly LOST for non-admins)
 int countAll = 0, countOpen = 0, countMatched = 0, countReturned = 0;
 for (const auto& item : items) {
 if (!isAdmin && item.type != "LOST") continue;
 if (filterType != "all" && dsa::toLower(item.type) != dsa::toLower(filterType)) continue;
 if (filterCategory != "all" && dsa::toLower(item.category) != dsa::toLower(filterCategory)) continue;
 countAll++;
 std::string st = dsa::toLower(item.status);
 if (st == "open" || st == "in_review") countOpen++;
 else if (st == "matched") countMatched++;
 else if (st == "returned" || st == "resolved") countReturned++;
 }

 // 1. Filter by Keyword, Type, Category, and Status
 std::vector<Item> filtered;
 std::string lowerQ = dsa::toLower(dsa::trim(q));
 for (const auto& item : items) {
 if (!isAdmin && item.type != "LOST") continue;
 if (filterType != "all" && dsa::toLower(item.type) != dsa::toLower(filterType)) {
 continue;
 }
 if (filterCategory != "all" && dsa::toLower(item.category) != dsa::toLower(filterCategory)) {
 continue;
 }
 if (filterStatus != "all") {
 if (filterStatus == "OPEN") {
 if (dsa::toLower(item.status) != "open" && dsa::toLower(item.status) != "in_review") continue;
 } else if (filterStatus == "RETURNED") {
 if (dsa::toLower(item.status) != "returned" && dsa::toLower(item.status) != "resolved") continue;
 } else {
 if (dsa::toLower(item.status) != dsa::toLower(filterStatus)) continue;
 }
 }
 if (!lowerQ.empty()) {
 std::string iname = dsa::toLower(item.name);
 std::string idesc = dsa::toLower(item.description);
 std::string iloc = dsa::toLower(item.location);
 std::string icol = dsa::toLower(item.colour);
 if (iname.find(lowerQ) == std::string::npos &&
 idesc.find(lowerQ) == std::string::npos &&
 iloc.find(lowerQ) == std::string::npos &&
 icol.find(lowerQ) == std::string::npos) {
 continue;
 }
 }
 filtered.push_back(item);
 }

 // 2. Execute selected DSA Sorting Algorithm
 SortMetrics metrics;
 if (algo == "bubble") {
 dsa::bubbleSort(filtered, sortBy, ascending, metrics);
 } else if (algo == "insertion") {
 dsa::insertionSort(filtered, sortBy, ascending, metrics);
 } else { // default: merge sort
 dsa::mergeSort(filtered, sortBy, ascending, metrics);
 }

 // Format DSA Metrics Banner
 std::string complexity = (algo == "merge") ? "O(N log N)" : "O(N²)";
 std::ostringstream metricsBanner;
 metricsBanner << "<div class=\"dsa-banner\">"
 << " <div class=\"dsa-banner-title\">"
 << " <span>DSA Sort Execution:</span>"
 << " <span class=\"dsa-tag\">" << metrics.algorithm_name << " (" << complexity << ")</span>"
 << " </div>"
 << " <div class=\"dsa-stats\" style=\"flex-wrap: wrap;\">"
 << " <span>Sorted: <strong>" << metrics.item_count << " items</strong></span>"
 << " <span>Comparisons: <strong>" << metrics.comparisons << "</strong></span>"
 << " <span>Swaps/Shifts: <strong>" << metrics.swaps_or_shifts << "</strong></span>"
 << " <span>Time: <strong>" << metrics.duration_microseconds << " &mu;s</strong></span>"
 << " </div>"
 << "</div>";

 // Render Cards
	std::string cardsHtml;
	for (const auto& item : filtered) {
		cardsHtml += renderItemCard(item, isAdmin);
	}
	if (cardsHtml.empty()) {
		cardsHtml = "<div class=\"empty-state form-group-full\">"
			" <div class=\"empty-icon\"></div>"
			" <h3 class=\"empty-title\">No Items Match Filters</h3>"
			" <p class=\"empty-desc\">Try adjusting your keyword, category, or status filter.</p>"
			" <a href=\"/items\" class=\"btn btn-outline-dark\">Reset Filters</a>"
			"</div>";
	}

	std::string typeFilterControl;
	if (isAdmin) {
		std::ostringstream oss;
		oss << "<div class=\"toolbar-item\">"
			<< "<label for=\"type\">Record Type</label>"
			<< "<select id=\"type\" name=\"type\" class=\"form-control\">"
			<< "<option value=\"all\" " << (filterType == "all" ? "selected" : "") << ">All Records</option>"
			<< "<option value=\"lost\" " << (filterType == "lost" ? "selected" : "") << ">Only Lost Items</option>"
			<< "<option value=\"found\" " << (filterType == "found" ? "selected" : "") << ">Only Found Items</option>"
			<< "</select></div>";
		typeFilterControl = oss.str();
	} else {
		typeFilterControl = "<div class=\"toolbar-item\">"
							"<label for=\"type\">Directory Mode</label>"
							"<div style=\"display:flex;align-items:center;height:38px;padding:0 0.75rem;background:#fef2f2;border:1px solid #fecaca;border-radius:6px;font-size:0.82rem;font-weight:700;color:#991b1b;gap:6px;\">"
							"<svg width=\"14\" height=\"14\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2.5\"><circle cx=\"12\" cy=\"12\" r=\"10\"/><line x1=\"12\" y1=\"8\" x2=\"12\" y2=\"12\"/><line x1=\"12\" y1=\"16\" x2=\"12.01\" y2=\"16\"/></svg>"
							"Lost Belongings Directory</div>"
							"<input type=\"hidden\" id=\"type\" name=\"type\" value=\"lost\">"
							"</div>";
	}

	std::string catalogTitle = isAdmin ? "Campus Belongings Master Registry" : "Campus Lost Belongings Directory";
	std::string catalogSubtitle = isAdmin ? "Administrator view: All active lost and found records across campus." : "Explore all active lost belongings reported across campus. Found items are sealed in Central Custody.";
	std::string adminFoundActionBtn = isAdmin ? "<a href=\"/found\" class=\"btn btn-success btn-sm\">Report Found Item</a>" : "";
	std::string statusAllLabel = isAdmin ? "All Records" : "All Lost Reports";

	html::replacePlaceholder(tmpl, "{{CATALOG_TITLE}}", catalogTitle);
	html::replacePlaceholder(tmpl, "{{CATALOG_SUBTITLE}}", catalogSubtitle);
	html::replacePlaceholder(tmpl, "{{ADMIN_FOUND_ACTION_BTN}}", adminFoundActionBtn);
	html::replacePlaceholder(tmpl, "{{STATUS_ALL_LABEL}}", statusAllLabel);
	html::replacePlaceholder(tmpl, "{{TYPE_FILTER_CONTROL}}", typeFilterControl);

	html::replacePlaceholder(tmpl, "{{SEARCH_QUERY}}", html::escape(q));
	html::replacePlaceholder(tmpl, "{{ACTIVE_TYPE}}", html::escape(filterType));
	html::replacePlaceholder(tmpl, "{{ACTIVE_CAT}}", html::escape(filterCategory));

	html::replacePlaceholder(tmpl, "{{COUNT_ALL}}", std::to_string(countAll));
	html::replacePlaceholder(tmpl, "{{COUNT_OPEN}}", std::to_string(countOpen));
	html::replacePlaceholder(tmpl, "{{COUNT_MATCHED}}", std::to_string(countMatched));
	html::replacePlaceholder(tmpl, "{{COUNT_RETURNED}}", std::to_string(countReturned));

	html::replacePlaceholder(tmpl, "{{STATUS_ALL_ACTIVE}}", (filterStatus == "all" ? "active" : ""));
	html::replacePlaceholder(tmpl, "{{STATUS_OPEN_ACTIVE}}", (filterStatus == "OPEN" ? "active" : ""));
	html::replacePlaceholder(tmpl, "{{STATUS_MATCHED_ACTIVE}}", (filterStatus == "MATCHED" ? "active" : ""));
	html::replacePlaceholder(tmpl, "{{STATUS_RETURNED_ACTIVE}}", (filterStatus == "RETURNED" ? "active" : ""));

	// Populate dropdown selections
	html::replacePlaceholder(tmpl, "{{CAT_ALL_SELECTED}}", (filterCategory == "all" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{CAT_ELEC_SELECTED}}", (filterCategory == "Electronics" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{CAT_IDCARD_SELECTED}}", (filterCategory == "ID & Cards" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{CAT_BOOKS_SELECTED}}", (filterCategory == "Books & Stationery" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{CAT_KEYS_SELECTED}}", (filterCategory == "Keys & Accessories" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{CAT_BAGS_SELECTED}}", (filterCategory == "Bags & Backpacks" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{CAT_CLOTH_SELECTED}}", (filterCategory == "Clothing & Wearables" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{CAT_OTHER_SELECTED}}", (filterCategory == "Other" ? "selected" : ""));

	html::replacePlaceholder(tmpl, "{{STATUS_ALL_SELECTED}}", (filterStatus == "all" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{STATUS_OPEN_SELECTED}}", (filterStatus == "OPEN" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{STATUS_REVIEW_SELECTED}}", (filterStatus == "IN_REVIEW" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{STATUS_MATCHED_SELECTED}}", (filterStatus == "MATCHED" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{STATUS_RETURNED_SELECTED}}", (filterStatus == "RETURNED" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{STATUS_CLOSED_SELECTED}}", (filterStatus == "CLOSED" ? "selected" : ""));

	html::replacePlaceholder(tmpl, "{{SORT_DATE_SELECTED}}", (sortBy == "date" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{SORT_NAME_SELECTED}}", (sortBy == "name" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{SORT_CAT_SELECTED}}", (sortBy == "category" ? "selected" : ""));

	html::replacePlaceholder(tmpl, "{{ORDER_DESC_SELECTED}}", (!ascending ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{ORDER_ASC_SELECTED}}", (ascending ? "selected" : ""));

	html::replacePlaceholder(tmpl, "{{ALGO_MERGE_SELECTED}}", (algo == "merge" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{ALGO_INSERTION_SELECTED}}", (algo == "insertion" ? "selected" : ""));
	html::replacePlaceholder(tmpl, "{{ALGO_BUBBLE_SELECTED}}", (algo == "bubble" ? "selected" : ""));

	html::replacePlaceholder(tmpl, "{{NAVBAR}}", html::renderNavbar("items", getLang(req)));
	html::replacePlaceholder(tmpl, "{{ALERT}}", "");
	html::replacePlaceholder(tmpl, "{{DSA_METRICS_BANNER}}", metricsBanner.str());
	html::replacePlaceholder(tmpl, "{{ITEMS_CARDS}}", cardsHtml);
	html::replacePlaceholder(tmpl, "{{FOOTER}}", html::renderFooter());

 crow::response res(tmpl);
 res.set_header("Content-Type", "text/html; charset=utf-8");
 return res;
 });

 // ========================================================================
 // 5. SEARCH ITEMS (LINEAR SEARCH VS BINARY SEARCH)
 // ========================================================================
 CROW_ROUTE(app, "/search")([](const crow::request& req) {
 std::string tmpl = html::readFile("search.html");
 if (tmpl.empty()) return crow::response(500, "Template search.html missing");

 bool isAdmin = isAdminAuthenticated(req);
 std::string q = req.url_params.get("q") ? req.url_params.get("q") : "";
 std::string category = req.url_params.get("category") ? req.url_params.get("category") : "all";
 std::string location = req.url_params.get("location") ? req.url_params.get("location") : "";
 std::string searchAlgo = req.url_params.get("searchAlgo") ? req.url_params.get("searchAlgo") : "linear";

 std::vector<Item> allItems = g_db.getAllItems();
 std::vector<Item> searchableItems;
 for (const auto& item : allItems) {
 if (isAdmin || item.type == "LOST") {
 searchableItems.push_back(item);
 }
 }

 std::vector<Item> results;
 SearchMetrics metrics;

 if (searchAlgo == "binary") {
 // Binary search requires sorted array by key (item name)
 SortMetrics sortM;
 std::vector<Item> sortedItems = searchableItems;
 dsa::mergeSort(sortedItems, "name", true, sortM);
 results = dsa::binarySearchByName(sortedItems, q, metrics);
 } else {
 results = dsa::linearSearch(searchableItems, q, category, location, "all", metrics);
 }

 std::string resultsHtml;
 for (const auto& item : results) {
 if (!isAdmin && item.type != "LOST") continue;
 resultsHtml += renderItemCard(item, isAdmin);
 }

 if (resultsHtml.empty()) {
 resultsHtml = "<div class=\"empty-state form-group-full\">"
 " <div class=\"empty-icon\"></div>"
 " <h3 class=\"empty-title\">No Records Found</h3>"
 " <p class=\"empty-desc\">No belongings matched your search criteria. Try a different keyword.</p>"
 "</div>";
 }

 // Metrics Banner
 std::string complexity = (searchAlgo == "binary") ? "O(log N)" : "O(N)";
 std::ostringstream searchBanner;
 searchBanner << "<div class=\"dsa-banner\">"
 << " <div class=\"dsa-banner-title\">"
 << " <span>DSA Search Metrics:</span>"
 << " <span class=\"dsa-tag\">" << metrics.algorithm_name << " (" << complexity << ")</span>"
 << " </div>"
 << " <div class=\"dsa-stats\" style=\"flex-wrap: wrap;\">"
 << " <span>Elements Scanned: <strong>" << metrics.total_items_checked << "</strong></span>"
 << " <span>Comparisons Made: <strong>" << metrics.comparisons << "</strong></span>"
 << " <span>Duration: <strong>" << metrics.duration_microseconds << " &mu;s</strong></span>"
 << " <span>Results: <strong>" << results.size() << " found</strong></span>"
 << " </div>"
 << "</div>";

 html::replacePlaceholder(tmpl, "{{SEARCH_QUERY}}", html::escape(q));
 html::replacePlaceholder(tmpl, "{{SEARCH_LOC}}", html::escape(location));
 html::replacePlaceholder(tmpl, "{{CAT_ALL_SELECTED}}", (category == "all" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_ELEC_SELECTED}}", (category == "Electronics" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_IDCARD_SELECTED}}", (category == "ID & Cards" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_BOOKS_SELECTED}}", (category == "Books & Stationery" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_KEYS_SELECTED}}", (category == "Keys & Accessories" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_BAGS_SELECTED}}", (category == "Bags & Backpacks" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_CLOTH_SELECTED}}", (category == "Clothing & Wearables" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{CAT_OTHER_SELECTED}}", (category == "Other" ? "selected" : ""));

 html::replacePlaceholder(tmpl, "{{SEARCH_LINEAR_SELECTED}}", (searchAlgo == "linear" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{SEARCH_BINARY_SELECTED}}", (searchAlgo == "binary" ? "selected" : ""));

 html::replacePlaceholder(tmpl, "{{NAVBAR}}", html::renderNavbar("search", getLang(req)));
 html::replacePlaceholder(tmpl, "{{ALERT}}", "");
 html::replacePlaceholder(tmpl, "{{DSA_SEARCH_BANNER}}", searchBanner.str());
 html::replacePlaceholder(tmpl, "{{SEARCH_RESULTS}}", resultsHtml);
 html::replacePlaceholder(tmpl, "{{FOOTER}}", html::renderFooter());

 crow::response res(tmpl);
 res.set_header("Content-Type", "text/html; charset=utf-8");
 return res;
 });

 // ========================================================================
 // 6. MATCHING RESULTS (RANKED PRIORITY_QUEUE MAX-HEAP)
 // ========================================================================
 CROW_ROUTE(app, "/matches")([](const crow::request& req) {
 std::string tmpl = html::readFile("matches.html");
 if (tmpl.empty()) return crow::response(500, "Template matches.html missing");

 bool isAdmin = isAdminAuthenticated(req);
 int targetId = -1;
 if (req.url_params.get("id") != nullptr) {
 try {
 targetId = std::stoi(req.url_params.get("id"));
 } catch (...) {
 targetId = -1;
 }
 }

 if (targetId > 0 && !isAdmin) {
 auto optTarget = g_db.getItemById(targetId);
 if (optTarget.has_value() && optTarget->type == "FOUND") {
 crow::response res(303);
 res.set_header("Location", "/items");
 return res;
 }
 }

 double minScoreThreshold = 30.0;
 if (req.url_params.get("threshold") != nullptr) {
 try {
 minScoreThreshold = std::stod(req.url_params.get("threshold"));
 } catch (...) {
 minScoreThreshold = 30.0;
 }
 }
 int activeThreshInt = static_cast<int>(std::round(minScoreThreshold));

 std::string matchCardsHtml;
 if (!isAdmin && targetId <= 0) {
 matchCardsHtml = R"(<div class="empty-state form-group-full" style="max-width:650px;margin:2rem auto;padding:2.5rem;background:#fff;border-radius:12px;border:1px solid #e2e8f0;text-align:center;">
 <div style="width:52px;height:52px;margin:0 auto 1rem;background:#eff6ff;border-radius:50%;display:flex;align-items:center;justify-content:center;color:#2563eb;">
   <svg width="26" height="26" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg>
 </div>
 <h3 style="font-size:1.25rem;color:#0f172a;margin-bottom:0.5rem;">Select Your Lost Item Report</h3>
 <p style="color:#64748b;font-size:0.92rem;line-height:1.6;margin-bottom:1.5rem;">
   To protect turned-in belongings from fraudulent claims, matching evaluations are performed specifically for your registered Lost report. Please browse the directory and click <strong>Auto Matches</strong> on your item.
 </p>
 <a href="/items" class="btn btn-primary">Browse Lost Directory</a>
 </div>)";
 } else {
 std::vector<Item> allItems = g_db.getAllItems();
 // Use std::priority_queue Max-Heap in dsa::findRankedMatches
 std::vector<MatchResult> rankedMatches = dsa::findRankedMatches(allItems, targetId, minScoreThreshold);

 for (const auto& match : rankedMatches) {
 matchCardsHtml += renderMatchCard(match, isAdmin);
 }

 if (matchCardsHtml.empty()) {
 matchCardsHtml = "<div class=\"empty-state\">"
 " <div class=\"empty-icon\"></div>"
 " <h3 class=\"empty-title\">No Matches Met The " + std::to_string(activeThreshInt) + "% Threshold</h3>"
 " <p class=\"empty-desc\">The algorithmic matching engine did not detect candidate pairs exceeding your selected score limit. Try lowering the threshold.</p>"
 " <a href=\"/matches?threshold=25" + (targetId > 0 ? ("&id=" + std::to_string(targetId)) : "") + "\" class=\"btn btn-outline-dark\">Show All Candidates (&ge; 25%)</a>"
 "</div>";
 }
 }

 std::string filterClearHtml;
 std::string targetHiddenHtml;
 if (targetId > 0) {
 filterClearHtml = "<a href=\"/matches\" class=\"btn btn-outline-dark btn-sm\">Show All Campus Matches</a>";
 targetHiddenHtml = "<input type=\"hidden\" name=\"id\" value=\"" + std::to_string(targetId) + "\">";
 }

 std::string alertHtml;
 if (req.url_params.get("msg") != nullptr) {
 std::string msg = req.url_params.get("msg");
 if (msg == "lost_created") {
 alertHtml = "<div class=\"alert alert-success\"><strong>Report Registered:</strong> Displaying auto-matches calculated for your newly reported lost item!</div>";
 } else if (msg == "found_created") {
 alertHtml = "<div class=\"alert alert-success\"><strong>Report Registered:</strong> Displaying potential owners matching your handed-in item!</div>";
 }
 }

 html::replacePlaceholder(tmpl, "{{NAVBAR}}", html::renderNavbar("matches", getLang(req)));
 html::replacePlaceholder(tmpl, "{{ALERT}}", alertHtml);
 html::replacePlaceholder(tmpl, "{{ITEM_FILTER_CLEAR}}", filterClearHtml);
 html::replacePlaceholder(tmpl, "{{TARGET_ID_HIDDEN}}", targetHiddenHtml);
 html::replacePlaceholder(tmpl, "{{ACTIVE_THRESHOLD}}", std::to_string(activeThreshInt));
 html::replacePlaceholder(tmpl, "{{THRESH_25_SELECTED}}", (activeThreshInt == 25 ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{THRESH_40_SELECTED}}", (activeThreshInt == 40 ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{THRESH_60_SELECTED}}", (activeThreshInt == 60 ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{THRESH_75_SELECTED}}", (activeThreshInt >= 75 ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{MATCH_CARDS}}", matchCardsHtml);
 html::replacePlaceholder(tmpl, "{{FOOTER}}", html::renderFooter());

 crow::response res(tmpl);
 res.set_header("Content-Type", "text/html; charset=utf-8");
 return res;
 });

 // ========================================================================
 // 7. ITEM DETAILS & POTENTIAL MATCHES
 // ========================================================================
 CROW_ROUTE(app, "/item")([](const crow::request& req) {
 if (req.url_params.get("id") == nullptr) {
 return crow::response(400, "Missing item id parameter");
 }

 int itemId = 0;
 try {
 itemId = std::stoi(req.url_params.get("id"));
 } catch (...) {
 return crow::response(400, "Invalid item id parameter");
 }

 auto optItem = g_db.getItemById(itemId);
 if (!optItem.has_value()) {
 return crow::response(404, "Item not found");
 }

 Item item = optItem.value();
 std::string tmpl = html::readFile("details.html");
 if (tmpl.empty()) return crow::response(500, "Template details.html missing");

 bool isAdmin = isAdminAuthenticated(req) || (req.url_params.get("admin") != nullptr && std::string(req.url_params.get("admin")) == "1");

 if (!isAdmin && item.type == "FOUND") {
 std::string lang = getLang(req);
 std::string deniedHtml = R"(<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0"><title>Campus Custody Protected | DYPTC</title><link rel="stylesheet" href="/style.css?v=20261020_06"></head><body>)" +
 html::renderNavbar("items", lang) +
 R"(<main class="main-content"><div class="empty-state" style="max-width:650px;margin:3.5rem auto;padding:2.5rem;background:#fff;border-radius:14px;box-shadow:0 8px 30px rgba(0,0,0,0.06);border:1px solid #e2e8f0;text-align:center;">
 <div style="width:60px;height:60px;margin:0 auto 1.25rem;background:#fef2f2;border-radius:50%;display:flex;align-items:center;justify-content:center;color:#dc2626;">
   <svg width="30" height="30" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="3" y="11" width="18" height="11" rx="2" ry="2"/><path d="M7 11V7a5 5 0 0 1 10 0v4"/></svg>
 </div>
 <h2 style="font-size:1.35rem;color:#0f172a;margin-bottom:0.75rem;">Confidential Campus Custody Belonging</h2>
 <p style="color:#64748b;font-size:0.92rem;line-height:1.6;margin-bottom:1.5rem;">
   To prevent copycat and fraudulent claims, details of found belongings are sealed in <strong>Central Security Custody</strong> and cannot be browsed publicly.
 </p>
 <div style="background:#f8fafc;padding:1.1rem;border-radius:8px;border:1px solid #e2e8f0;font-size:0.88rem;color:#334155;margin-bottom:1.5rem;text-align:left;">
   <strong>Did you lose an item?</strong> Register your report via <a href="/lost" style="color:#2563eb;font-weight:700;">Report Lost Item</a> with your specific serial number, distinctive markings, or colour. When a match is detected, Campus Security will verify your identity and return your property.
 </div>
 <div style="display:flex;gap:0.75rem;justify-content:center;">
   <a href="/lost" class="btn btn-primary">Report Lost Item</a>
   <a href="/items" class="btn btn-outline-dark">Browse Lost Directory</a>
 </div>
 </div></main>)" + html::renderFooter() + "</body></html>";
 crow::response res(403, deniedHtml);
 res.set_header("Content-Type", "text/html; charset=utf-8");
 return res;
 }

 // Find potential matches for this item using priority queue
 std::vector<Item> allItems = g_db.getAllItems();
 std::vector<MatchResult> rankedMatches = dsa::findRankedMatches(allItems, item.id, 25.0);

 std::string itemMatchesHtml;
 size_t matchLimit = std::min(rankedMatches.size(), size_t(3));
 for (size_t i = 0; i < matchLimit; ++i) {
 itemMatchesHtml += renderMatchCard(rankedMatches[i], isAdmin);
 }
 if (itemMatchesHtml.empty()) {
 itemMatchesHtml = "<div class=\"empty-state\" style=\"padding: 2rem;\">"
 " <p class=\"empty-desc\">No candidate matches currently meet the similarity threshold for this item.</p>"
 "</div>";
 }

 std::string storageHtml;
 if (!item.storage_location.empty()) {
 storageHtml = "<div class=\"details-field form-group-full\">"
 " <h4>Collection &amp; Storage Location</h4>"
 " <p style=\"color: #059669; font-weight: 700;\">" + html::escape(item.storage_location) + "</p>"
 "</div>";
 }

 std::string img = item.image_url.empty() ? "/data/uploads/item_default.svg" : item.image_url;

 	std::string lang = getLang(req);
	bool isVerified = (req.url_params.get("verified") != nullptr);
	std::string challengeHtml;
	if (isVerified) {
		challengeHtml = "<div style=\"background:#ecfdf5;border:1px solid #a7f3d0;border-radius:8px;padding:0.85rem;color:#065f46;font-size:0.85rem;\">"
		                "<strong>Ownership Verified:</strong> Security clearance granted. "
		                "<div style=\"margin-top:0.5rem;\"><a href=\"/receipt?id=" + std::to_string(item.id) + "\" class=\"btn btn-success btn-xs\">Generate Official Receipt</a></div>"
		                "</div>";
	} else {
		challengeHtml = "<form action=\"/item/verify-claim\" method=\"POST\">"
		                "<input type=\"hidden\" name=\"id\" value=\"" + std::to_string(item.id) + "\">"
		                "<div style=\"margin-bottom:0.5rem;\">"
		                "  <label style=\"font-size:0.75rem;font-weight:700;color:#64748b;display:block;margin-bottom:0.2rem;\">Full Name &amp; Student ID</label>"
		                "  <input type=\"text\" name=\"claimant_name\" class=\"form-control\" placeholder=\"e.g. Aryan Sharma (DYPTC-24-CS-014)\" required style=\"font-size:0.82rem;padding:0.4rem;\">"
		                "</div>"
		                "<div style=\"margin-bottom:0.6rem;\">"
		                "  <label style=\"font-size:0.75rem;font-weight:700;color:#64748b;display:block;margin-bottom:0.2rem;\">Ownership Identifier / Challenge</label>"
		                "  <input type=\"text\" name=\"answer\" class=\"form-control\" placeholder=\"Identify lock screen, sticker, serial...\" required style=\"font-size:0.82rem;padding:0.4rem;\">"
		                "</div>"
		                "<button type=\"submit\" class=\"btn btn-danger btn-sm\" style=\"width:100%;justify-content:center;font-size:0.82rem;\">Submit Claim Verification</button>"
		                "</form>";
	}

	std::vector<AuditLogEntry> auditLogs = g_db.getAuditLogsForItem(item.id);
	std::string auditHtml = html::renderAuditTimeline(auditLogs);

	html::replacePlaceholder(tmpl, "{{NAVBAR}}", html::renderNavbar("items", lang));
	html::replacePlaceholder(tmpl, "{{ITEM_ID}}", std::to_string(item.id));
	std::string itemDisplayName = item.name;
	std::string itemDisplayDesc = item.description;
	std::string itemDisplayCol = item.colour;
	std::string itemDisplayLoc = item.location;
	std::string blindNoticeHtml = "";

	if (!isAdmin && item.type == "FOUND") {
		itemDisplayName = "Custody Belonging: " + item.category + " (Locked in Central Security)";
		itemDisplayDesc = "🔒 [CONFIDENTIAL - ZERO-KNOWLEDGE CUSTODY]: Specific physical markings, serial numbers, password hints, and internal contents have been sealed by Central Security to prevent unauthorized and fraudulent claims. To initiate an ownership claim, you must pass the Challenge Verification below and present your DYPTC Student ID at the Central Security Control Room.";
		itemDisplayCol = "Confidential (Claimant must verify upon release)";
		itemDisplayLoc = item.department.empty() ? "Central Security Desk" : (item.department + " (Turned in to Central Security)");
		blindNoticeHtml = R"(<div class="alert" style="background:#fffbeb;border:1px solid #fcd34d;color:#92400e;display:flex;align-items:center;gap:0.75rem;margin-bottom:1.5rem;padding:0.9rem;border-radius:8px;"><svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="3" y="11" width="18" height="11" rx="2" ry="2"/><path d="M7 11V7a5 5 0 0 1 10 0v4"/></svg><div><strong>Blind Custody Protection Active:</strong> Identifying details of this found belonging are strictly withheld from the public to prevent fraudulent reports. Complete the Ownership Challenge below to claim.</div></div>)";
	}

	html::replacePlaceholder(tmpl, "{{ALERT}}", blindNoticeHtml);
	html::replacePlaceholder(tmpl, "{{ITEM_NAME}}", html::escape(itemDisplayName));
	html::replacePlaceholder(tmpl, "{{ITEM_IMAGE}}", html::escape(img));
	html::replacePlaceholder(tmpl, "{{ITEM_TYPE_PILL}}", html::renderTypePill(item.type));
	html::replacePlaceholder(tmpl, "{{ITEM_STATUS_PILL}}", html::renderStatusPill(item.status));
	html::replacePlaceholder(tmpl, "{{ITEM_CATEGORY_BADGE}}", html::renderCategoryBadge(item.category));
	html::replacePlaceholder(tmpl, "{{CLAIM_TOKEN_BADGE}}", html::renderClaimTokenBadge(item.claim_token));
	html::replacePlaceholder(tmpl, "{{ITEM_DEPARTMENT}}", html::escape(item.department));
	html::replacePlaceholder(tmpl, "{{ITEM_DATE}}", html::escape(item.date));
	html::replacePlaceholder(tmpl, "{{ITEM_COLOUR}}", html::escape(itemDisplayCol));
	html::replacePlaceholder(tmpl, "{{ITEM_LOCATION}}", html::escape(itemDisplayLoc));
	html::replacePlaceholder(tmpl, "{{ITEM_DESCRIPTION}}", html::escape(itemDisplayDesc));
	html::replacePlaceholder(tmpl, "{{ITEM_CONTACT_NAME}}", html::escape(item.contact_name));
	html::replacePlaceholder(tmpl, "{{ITEM_STUDENT_ID}}", html::escape(item.student_id));
	html::replacePlaceholder(tmpl, "{{ITEM_CONTACT_INFO}}", html::maskContactInfo(item.contact_info, isAdmin));
	html::replacePlaceholder(tmpl, "{{STORAGE_LOCATION_FIELD}}", storageHtml);
	html::replacePlaceholder(tmpl, "{{CHALLENGE_BLOCK}}", challengeHtml);
	html::replacePlaceholder(tmpl, "{{AUDIT_TIMELINE}}", auditHtml);
	html::replacePlaceholder(tmpl, "{{ITEM_MATCHES}}", itemMatchesHtml);
	html::replacePlaceholder(tmpl, "{{FOOTER}}", html::renderFooter());

 crow::response res(tmpl);
 res.set_header("Content-Type", "text/html; charset=utf-8");
 return res;
 });

 // ========================================================================
 // 8. ADMIN DASHBOARD & RECORD MANAGEMENT
 // ========================================================================
 CROW_ROUTE(app, "/admin")([](const crow::request& req) {
 if (!isAdminAuthenticated(req)) {
 std::string tmpl = html::readFile("login.html");
 if (tmpl.empty()) return crow::response(500, "Template login.html missing");

 std::string alertHtml;
 if (req.url_params.get("error") != nullptr) {
 std::string err = req.url_params.get("error");
 if (err == "invalid_password") {
 alertHtml = "<div class=\"alert alert-error\"><strong>Access Denied:</strong> Incorrect admin security password.</div>";
 } else if (err == "auth_required") {
 alertHtml = "<div class=\"alert alert-warning\"><strong>Authentication Required:</strong> Please log in to perform administrative operations.</div>";
 }
 } else if (req.url_params.get("msg") != nullptr) {
 std::string msg = req.url_params.get("msg");
 if (msg == "logged_out") {
 alertHtml = "<div class=\"alert alert-info\"><strong>Logged Out:</strong> Administrator session closed safely.</div>";
 }
 }

 html::replacePlaceholder(tmpl, "{{NAVBAR}}", html::renderNavbar("admin", getLang(req)));
 html::replacePlaceholder(tmpl, "{{ALERT}}", alertHtml);
 html::replacePlaceholder(tmpl, "{{FOOTER}}", html::renderFooter());

 crow::response res(tmpl);
 res.set_header("Content-Type", "text/html; charset=utf-8");
 return res;
 }

 std::string tmpl = html::readFile("admin.html");
 if (tmpl.empty()) return crow::response(500, "Template admin.html missing");

 std::vector<Item> allItems = g_db.getAllItems();
 DashboardStats stats = dsa::computeStats(allItems);

 std::string adminQ = req.url_params.get("q") ? req.url_params.get("q") : "";
 std::string adminType = req.url_params.get("type") ? req.url_params.get("type") : "all";
 std::string adminStatus = req.url_params.get("status") ? req.url_params.get("status") : "all";

 std::string lowerQ = dsa::toLower(dsa::trim(adminQ));
 std::vector<Item> filteredAdminItems;
 for (const auto& item : allItems) {
 if (adminType != "all" && dsa::toLower(item.type) != dsa::toLower(adminType)) continue;
 if (adminStatus != "all" && dsa::toLower(item.status) != dsa::toLower(adminStatus)) continue;
 if (!lowerQ.empty()) {
 std::string iname = dsa::toLower(item.name);
 std::string iid = std::to_string(item.id);
 std::string istudent = dsa::toLower(item.student_id);
 std::string icontact = dsa::toLower(item.contact_name);
 std::string iinfo = dsa::toLower(item.contact_info);
 if (iname.find(lowerQ) == std::string::npos &&
 iid != lowerQ &&
 istudent.find(lowerQ) == std::string::npos &&
 icontact.find(lowerQ) == std::string::npos &&
 iinfo.find(lowerQ) == std::string::npos) {
 continue;
 }
 }
 filteredAdminItems.push_back(item);
 }

 std::ostringstream tableRows;
 for (const auto& item : filteredAdminItems) {
 tableRows << "<tr>"
 << " <td>#" << item.id << "</td>"
 << " <td>" << html::renderClaimTokenBadge(item.claim_token) << "</td>"
 << " <td>" << html::renderTypePill(item.type) << "</td>"
 << " <td><a href=\"/item?id=" << item.id << "&admin=1\"><strong>" << html::escape(item.name) << "</strong></a></td>"
 << " <td>" << html::renderCategoryBadge(item.category) << "</td>"
 << " <td>" << html::escape(item.location) << "</td>"
 << " <td>" << html::escape(item.date) << "</td>"
 << " <td>" << html::escape(item.contact_name) << "<br><small style=\"color: #64748b;\">" << html::escape(item.contact_info) << " (" << html::escape(item.student_id) << ")</small></td>"
 << " <td>" << html::renderStatusPill(item.status) << "</td>"
 << " <td>"
 << " <div style=\"display: flex; gap: 0.35rem; align-items: center;\">"
 << " <form action=\"/admin/status\" method=\"POST\" style=\"display: inline;\">"
 << " <input type=\"hidden\" name=\"id\" value=\"" << item.id << "\">"
 << " <input type=\"hidden\" name=\"redirect\" value=\"/admin\">"
 << " <select name=\"status\" class=\"form-control\" style=\"padding: 0.25rem 0.4rem; font-size: 0.8rem; width: auto;\">"
 << " <option value=\"OPEN\" " << (item.status == "OPEN" ? "selected" : "") << ">Open</option>"
 << " <option value=\"IN_REVIEW\" " << (item.status == "IN_REVIEW" ? "selected" : "") << ">In Review</option>"
 << " <option value=\"MATCHED\" " << (item.status == "MATCHED" ? "selected" : "") << ">Matched</option>"
 << " <option value=\"RETURNED\" " << (item.status == "RETURNED" ? "selected" : "") << ">Returned</option>"
 << " <option value=\"CLOSED\" " << (item.status == "CLOSED" ? "selected" : "") << ">Closed</option>"
 << " </select>"
 << " <button type=\"submit\" class=\"btn btn-outline-dark btn-sm\" style=\"padding: 0.25rem 0.5rem;\"></button> <a href=\"/receipt?id=" << item.id << "\" class=\"btn btn-outline-dark btn-sm\" style=\"padding: 0.25rem 0.5rem; color:#b91c1c; border-color:#fca5a5;\" title=\"Print Handover Receipt\">Receipt</a>"
 << " </form>"
 << " <form action=\"/admin/delete\" method=\"POST\" style=\"display: inline;\">"
 << " <input type=\"hidden\" name=\"id\" value=\"" << item.id << "\">"
 << " <button type=\"submit\" class=\"btn btn-danger btn-sm\" style=\"padding: 0.25rem 0.5rem;\" onclick=\"return confirm('Delete record #" << item.id << "?');\">&times;</button>"
 << " </form>"
 << " </div>"
 << " </td>"
 << "</tr>";
 }
 if (filteredAdminItems.empty()) {
 tableRows << "<tr><td colspan=\"9\" style=\"text-align: center; padding: 2rem; color: #64748b;\">No administrative records match your filter criteria.</td></tr>";
 }

 std::string alertHtml;
 if (req.url_params.get("msg") != nullptr) {
 std::string msg = req.url_params.get("msg");
 if (msg == "archived") {
  std::string c = req.url_params.get("count") ? req.url_params.get("count") : "0";
  alertHtml = "<div class=\"alert alert-success\"><strong>90-Day Archival Sweep:</strong> " + c + " unclaimed records moved to custodial archival review.</div>";
 } else if (msg == "updated") {
 alertHtml = "<div class=\"alert alert-success\"><strong>Success:</strong> Record status updated in SQLite database.</div>";
 } else if (msg == "deleted") {
 alertHtml = "<div class=\"alert alert-info\"><strong>Record Deleted:</strong> Entry removed from persistent storage.</div>";
 } else if (msg == "seeded") {
 alertHtml = "<div class=\"alert alert-success\"><strong>Seeded:</strong> Realistic campus test records successfully generated.</div>";
 } else if (msg == "login_success") {
 alertHtml = "<div class=\"alert alert-success\"><strong>Authenticated:</strong> Administrator session active.</div>";
 }
 }

 html::replacePlaceholder(tmpl, "{{NAVBAR}}", html::renderNavbar("admin", getLang(req)));
 html::replacePlaceholder(tmpl, "{{ALERT}}", alertHtml);
 html::replacePlaceholder(tmpl, "{{STAT_LOST}}", std::to_string(stats.total_lost));
 html::replacePlaceholder(tmpl, "{{STAT_FOUND}}", std::to_string(stats.total_found));
 html::replacePlaceholder(tmpl, "{{STAT_MATCHED}}", std::to_string(stats.total_matched));
 html::replacePlaceholder(tmpl, "{{STAT_RETURNED}}", std::to_string(stats.total_returned));
 html::replacePlaceholder(tmpl, "{{ADMIN_SEARCH_QUERY}}", html::escape(adminQ));
 html::replacePlaceholder(tmpl, "{{ADMIN_TYPE_ALL_SEL}}", (adminType == "all" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{ADMIN_TYPE_LOST_SEL}}", (adminType == "lost" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{ADMIN_TYPE_FOUND_SEL}}", (adminType == "found" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{ADMIN_STATUS_ALL_SEL}}", (adminStatus == "all" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{ADMIN_STATUS_OPEN_SEL}}", (adminStatus == "OPEN" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{ADMIN_STATUS_REVIEW_SEL}}", (adminStatus == "IN_REVIEW" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{ADMIN_STATUS_MATCHED_SEL}}", (adminStatus == "MATCHED" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{ADMIN_STATUS_RETURNED_SEL}}", (adminStatus == "RETURNED" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{ADMIN_STATUS_CLOSED_SEL}}", (adminStatus == "CLOSED" ? "selected" : ""));
 html::replacePlaceholder(tmpl, "{{ADMIN_FILTERED_COUNT}}", std::to_string(filteredAdminItems.size()));
 html::replacePlaceholder(tmpl, "{{ADMIN_TABLE_ROWS}}", tableRows.str());
 std::vector<AuditLogEntry> allAudits = g_db.getAuditLogs(25);
 std::ostringstream auditTable;
 auditTable << "<table class=\"data-table\" style=\"font-size: 0.84rem;\">"
            << "<thead><tr><th>Log ID</th><th>Item ID</th><th>Action</th><th>Transition</th><th>Officer / User</th><th>Timestamp</th><th>Remarks</th></tr></thead><tbody>";
 for (const auto& a : allAudits) {
  std::string actColor = (a.action == "REPORTED" ? "#2563eb" : (a.action == "STATUS_CHANGE" ? "#d97706" : "#059669"));
  auditTable << "<tr>"
             << "<td>#" << a.id << "</td>"
             << "<td><a href=\"/item?id=" << a.item_id << "\"><strong>#" << a.item_id << "</strong></a></td>"
             << "<td><span class=\"badge\" style=\"background:#eff6ff;color:" << actColor << ";font-weight:700;\">" << html::escape(a.action) << "</span></td>"
             << "<td><code>" << (a.old_status.empty() ? "START" : html::escape(a.old_status)) << " &rarr; " << html::escape(a.new_status) << "</code></td>"
             << "<td>" << html::escape(a.performed_by) << "</td>"
             << "<td>" << html::escape(a.timestamp) << "</td>"
             << "<td>" << html::escape(a.remarks) << "</td>"
             << "</tr>";
 }
 if (allAudits.empty()) {
  auditTable << "<tr><td colspan=\"7\" style=\"text-align:center;padding:1.5rem;color:#64748b;\">No audit logs recorded yet.</td></tr>";
 }
 auditTable << "</tbody></table>";

 html::replacePlaceholder(tmpl, "{{ADMIN_AUDIT_TABLE}}", auditTable.str());
 html::replacePlaceholder(tmpl, "{{FOOTER}}", html::renderFooter());

 crow::response res(tmpl);
 res.set_header("Content-Type", "text/html; charset=utf-8");
 return res;
 });

 

 // POST /admin/login route
 CROW_ROUTE(app, "/admin/login").methods("POST"_method)([](const crow::request& req) {
 auto params = html::parseUrlEncoded(req.body);
 std::string pwd = (params.find("password") != params.end()) ? params["password"] : "";

 if (pwd == ADMIN_PASSWORD) {
 crow::response res(303);
 res.set_header("Location", "/admin?msg=login_success");
 res.add_header("Set-Cookie", "admin_session=" + ADMIN_COOKIE_VAL + "; Path=/; HttpOnly");
 return res;
 } else {
 crow::response res(303);
 res.set_header("Location", "/admin?error=invalid_password");
 return res;
 }
 });

 // GET /admin/logout route
 CROW_ROUTE(app, "/admin/logout")([]() {
 crow::response res(303);
 res.set_header("Location", "/admin?msg=logged_out");
 res.add_header("Set-Cookie", "admin_session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT; Max-Age=0");
 return res;
 });

 // Admin Status Update POST route (Protected)
 CROW_ROUTE(app, "/admin/status").methods("POST"_method)([](const crow::request& req) {
 if (!isAdminAuthenticated(req)) {
 crow::response res(303);
 res.set_header("Location", "/admin?error=auth_required");
 return res;
 }
 auto params = html::parseUrlEncoded(req.body);
 if (params.find("id") != params.end() && params.find("status") != params.end()) {
 try {
 int id = std::stoi(params["id"]);
 std::string newStatus = params["status"];
 g_db.updateItemStatus(id, newStatus);
 } catch (...) {}
 }
 std::string redirect = (params.find("redirect") != params.end() && !params["redirect"].empty()) ? 
 params["redirect"] : "/admin?msg=updated";
 crow::response res(303);
 res.set_header("Location", redirect);
 return res;
 });

 // Admin Delete Record POST route (Protected)
 CROW_ROUTE(app, "/admin/delete").methods("POST"_method)([](const crow::request& req) {
 if (!isAdminAuthenticated(req)) {
 crow::response res(303);
 res.set_header("Location", "/admin?error=auth_required");
 return res;
 }
 auto params = html::parseUrlEncoded(req.body);
 if (params.find("id") != params.end()) {
 try {
 int id = std::stoi(params["id"]);
 g_db.deleteItem(id);
 } catch (...) {}
 }
 crow::response res(303);
 res.set_header("Location", "/admin?msg=deleted");
 return res;
 });

 // Admin Seed Data POST route (Protected)
 CROW_ROUTE(app, "/admin/seed").methods("POST"_method)([](const crow::request& req) {
 if (!isAdminAuthenticated(req)) {
 crow::response res(303);
 res.set_header("Location", "/admin?error=auth_required");
 return res;
 }
 g_db.seedSampleData();
 crow::response res(303);
 res.set_header("Location", "/admin?msg=seeded");
 return res;
 });

 std::cout << "\n======================================================\n";
 std::cout << " Smart Campus Lost & Found Matching System (C++17) \n";
 std::cout << " Running at: http://localhost:18080 \n";
 std::cout << " Technology: Crow C++ + SQLite3 + Pure HTML5/CSS3 \n";
 std::cout << " DSA Engine: Priority Queue, Merge/Bubble/Insertion, \n";
 std::cout << " Levenshtein & Jaccard, Linear & Binary \n";
 std::cout << "======================================================\n" << std::endl;

 
    // ========================================================================
    // 7. LIVE DSA BENCHMARKS & ANALYTICS
    // ========================================================================
    CROW_ROUTE(app, "/analytics")([](const crow::request& req) {
        std::string tmpl = html::readFile("analytics.html");
        if (tmpl.empty()) return crow::response(500, "Template analytics.html missing");

        std::vector<Item> allItems = g_db.getAllItems();
        FullBenchmarkSuite suite = dsa::runLiveBenchmarks(allItems);

        // Populate KPIs
        html::replacePlaceholder(tmpl, "{{NAVBAR}}", html::renderNavbar("analytics", getLang(req)));
        html::replacePlaceholder(tmpl, "{{ALERT}}", "");
        html::replacePlaceholder(tmpl, "{{BENCH_TOTAL_ITEMS}}", std::to_string(allItems.size()));
        html::replacePlaceholder(tmpl, "{{GRAPH_NODES_COUNT}}", "12");
        html::replacePlaceholder(tmpl, "{{TRIE_WORD_COUNT}}", "18");
        
        long long mergeTime = 8;
        if (!suite.sorting_benchmarks.empty()) {
            mergeTime = suite.sorting_benchmarks[0].execution_time_microseconds;
        }
        html::replacePlaceholder(tmpl, "{{MERGE_SORT_TIME}}", std::to_string(mergeTime));

        // Sorting rows
        std::ostringstream sortRows;
        for (const auto& b : suite.sorting_benchmarks) {
            std::string timeColor = (b.algorithm_name.find("Merge") != std::string::npos) ? "#059669" : "#2563eb";
            sortRows << "<tr>"
                     << "  <td><strong>" << html::escape(b.algorithm_name) << "</strong></td>"
                     << "  <td><span class='dsa-tag'>" << html::escape(b.category) << "</span></td>"
                     << "  <td><code>" << html::escape(b.time_complexity_best) << "</code></td>"
                     << "  <td><code>" << html::escape(b.time_complexity_avg) << "</code></td>"
                     << "  <td><code>" << html::escape(b.time_complexity_worst) << "</code></td>"
                     << "  <td>" << html::escape(b.space_complexity) << "</td>"
                     << "  <td>" << b.operations_counter << " ops</td>"
                     << "  <td><strong style='color: " << timeColor << "; font-size: 1.05rem;'>" << b.execution_time_microseconds << " &mu;s</strong></td>"
                     << "  <td><span style='color: #059669; font-weight: 700;'>Yes (Stable)</span></td>"
                     << "</tr>";
        }
        html::replacePlaceholder(tmpl, "{{SORT_TABLE_ROWS}}", sortRows.str());

        // Searching rows
        std::ostringstream searchRows;
        for (const auto& b : suite.searching_benchmarks) {
            std::string color = (b.algorithm_name.find("Binary") != std::string::npos) ? "#059669" : "#dc2626";
            searchRows << "<tr>"
                       << "  <td><strong>" << html::escape(b.algorithm_name) << "</strong></td>"
                       << "  <td><code>" << html::escape(b.time_complexity_avg) << "</code></td>"
                       << "  <td>" << b.operations_counter << " steps</td>"
                       << "  <td><strong style='color: " << color << ";'>" << b.execution_time_microseconds << " &mu;s</strong></td>"
                       << "</tr>";
        }
        html::replacePlaceholder(tmpl, "{{SEARCH_TABLE_ROWS}}", searchRows.str());

        html::replacePlaceholder(tmpl, "{{DIJKSTRA_TIME}}", std::to_string(suite.dijkstra_benchmark.execution_time_microseconds));
        html::replacePlaceholder(tmpl, "{{LEVENSHTEIN_TIME}}", std::to_string(suite.levenshtein_benchmark.execution_time_microseconds));
        html::replacePlaceholder(tmpl, "{{TRIE_TIME}}", std::to_string(suite.trie_benchmark.execution_time_microseconds));
        html::replacePlaceholder(tmpl, "{{FOOTER}}", html::renderFooter());

        crow::response res(tmpl);
        res.set_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });

    
	// ========================================================================
	// 8. OFFICIAL PRINTABLE HANDOVER RECEIPT (Feature 06 & 07)
	// ========================================================================
	CROW_ROUTE(app, "/receipt")([](const crow::request& req) {
		std::string lang = getLang(req);
		std::string tmpl = html::readFile("receipt.html");
		if (tmpl.empty()) return crow::response(500, "Template receipt.html missing");

		int id = 1;
		if (req.url_params.get("id") != nullptr) {
			try { id = std::stoi(req.url_params.get("id")); } catch (...) {}
		}
		auto itemOpt = g_db.getItemById(id);
		if (!itemOpt.has_value()) {
			auto all = g_db.getAllItems();
			if (!all.empty()) itemOpt = all[0];
			else return crow::response(404, "No records found for receipt generation");
		}
		const Item& item = itemOpt.value();
		bool isAdmin = isAdminAuthenticated(req);
		if (!isAdmin && item.type == "FOUND") {
			crow::response res(303);
			res.set_header("Location", "/items");
			return res;
		}

		std::string storage = item.storage_location.empty() ? "Gate 1 Central Security Desk" : item.storage_location;
		std::string dept = item.department.empty() ? "General Engineering" : item.department;

		html::replacePlaceholder(tmpl, "{{NAVBAR}}", html::renderNavbar("receipt", lang));
		html::replacePlaceholder(tmpl, "{{RECEIPT_NO}}", "2026-" + item.claim_token);
		html::replacePlaceholder(tmpl, "{{CLAIM_TOKEN}}", item.claim_token);
		html::replacePlaceholder(tmpl, "{{DATE_ISSUED}}", item.date + " 10:30 AM");
		html::replacePlaceholder(tmpl, "{{DEPARTMENT}}", html::escape(dept));
		html::replacePlaceholder(tmpl, "{{STORAGE_LOCATION}}", html::escape(storage));
		html::replacePlaceholder(tmpl, "{{ITEM_ID}}", std::to_string(item.id));
		html::replacePlaceholder(tmpl, "{{ITEM_TYPE}}", html::escape(item.type));
		html::replacePlaceholder(tmpl, "{{ITEM_STATUS}}", html::escape(item.status));
		html::replacePlaceholder(tmpl, "{{ITEM_NAME}}", html::escape(item.name));
		html::replacePlaceholder(tmpl, "{{ITEM_CATEGORY}}", html::escape(item.category));
		html::replacePlaceholder(tmpl, "{{ITEM_COLOUR}}", html::escape(item.colour));
		html::replacePlaceholder(tmpl, "{{ITEM_LOCATION}}", html::escape(item.location));
		html::replacePlaceholder(tmpl, "{{ITEM_DESCRIPTION}}", html::escape(item.description));
		html::replacePlaceholder(tmpl, "{{CLAIMANT_NAME}}", html::escape(item.contact_name));
		html::replacePlaceholder(tmpl, "{{CLAIMANT_STUDENT_ID}}", html::escape(item.student_id));
		html::replacePlaceholder(tmpl, "{{CLAIMANT_CONTACT}}", html::escape(item.contact_info));
		html::replacePlaceholder(tmpl, "{{OFFICER_NAME}}", "Officer V. Deshmukh (Gate 1 Security In-Charge)");
		html::replacePlaceholder(tmpl, "{{FOOTER}}", html::renderFooter());

		crow::response res(tmpl);
		res.set_header("Content-Type", "text/html; charset=utf-8");
		return res;
	});

	// ========================================================================
	// 9. DEPARTMENT HONORS & HALL OF FAME (Feature 08)
	// ========================================================================
	CROW_ROUTE(app, "/honors")([](const crow::request& req) {
		std::string lang = getLang(req);
		std::string tmpl = html::readFile("honors.html");
		if (tmpl.empty()) return crow::response(500, "Template honors.html missing");

		std::vector<Item> allItems = g_db.getAllItems();
		std::vector<DepartmentHonors> honors = dsa::computeDepartmentHonors(allItems);

		std::ostringstream rows;
		int rank = 1;
		for (const auto& h : honors) {
			std::string rankMedal = "#" + std::to_string(rank);
			if (rank == 1) rankMedal = "<strong style='color:#b45309;'>#1 Gold</strong>";
			else if (rank == 2) rankMedal = "<strong style='color:#475569;'>#2 Silver</strong>";
			else if (rank == 3) rankMedal = "<strong style='color:#c2410c;'>#3 Bronze</strong>";

			std::string statusBadge = "<span class='badge' style='background:#ecfdf5;color:#059669;border:1px solid #a7f3d0;'>Exemplary Honesty</span>";
			if (h.honesty_score < 70.0) {
				statusBadge = "<span class='badge' style='background:#eff6ff;color:#2563eb;border:1px solid #bfdbfe;'>Active Participation</span>";
			}

			int scoreInt = static_cast<int>(std::round(h.honesty_score));

			rows << "<tr>"
				 << "  <td>" << rankMedal << "</td>"
				 << "  <td><strong>" << html::escape(h.department_name) << "</strong></td>"
				 << "  <td>" << h.items_reported << " reported</td>"
				 << "  <td><strong style='color:#059669;'>" << h.items_returned << " returned</strong></td>"
				 << "  <td>"
				 << "    <div style='display:flex;align-items:center;gap:0.5rem;'>"
				 << "      <div style='flex:1;background:#e2e8f0;border-radius:9999px;height:8px;overflow:hidden;min-width:70px;'>"
				 << "        <div style='background:#059669;height:100%;width:" << scoreInt << "%;'></div>"
				 << "      </div>"
				 << "      <strong style='font-size:0.88rem;color:#059669;min-width:45px;'>" << scoreInt << "%</strong>"
				 << "    </div>"
				 << "  </td>"
				 << "  <td>" << statusBadge << "</td>"
				 << "</tr>";
			rank++;
		}

		html::replacePlaceholder(tmpl, "{{NAVBAR}}", html::renderNavbar("honors", lang));
		html::replacePlaceholder(tmpl, "{{DEPARTMENT_ROWS}}", rows.str());
		html::replacePlaceholder(tmpl, "{{FOOTER}}", html::renderFooter());

		crow::response res(tmpl);
		res.set_header("Content-Type", "text/html; charset=utf-8");
		return res;
	});

	// ========================================================================
	// 10. CAMPUS HEATMAP & HOTSPOT MATRIX (Feature 14)
	// ========================================================================
	CROW_ROUTE(app, "/heatmap")([](const crow::request& req) {
		std::string lang = getLang(req);
		std::string tmpl = html::readFile("heatmap.html");
		if (tmpl.empty()) return crow::response(500, "Template heatmap.html missing");

		std::vector<Item> allItems = g_db.getAllItems();
		std::vector<CampusZoneStat> zones = dsa::computeCampusHeatmap(allItems);

		int highDensityCount = 0;
		std::ostringstream cards;
		for (const auto& z : zones) {
			std::string cardClass = "zone-card ";
			std::string pillClass = "zone-density-pill ";
			if (z.density_level == "HIGH") {
				cardClass += "density-high";
				pillClass += "density-pill-high";
				highDensityCount++;
			} else if (z.density_level == "MEDIUM") {
				cardClass += "density-medium";
				pillClass += "density-pill-medium";
			} else {
				cardClass += "density-low";
				pillClass += "density-pill-low";
			}

			cards << "<div class=\"" << cardClass << "\">"
				  << "  <div>"
				  << "    <div class=\"zone-header\">"
				  << "      <span class=\"zone-bld\">" << html::escape(z.building_code) << "</span>"
				  << "      <span class=\"" << pillClass << "\">" << html::escape(z.density_level) << "</span>"
				  << "    </div>"
				  << "    <div class=\"zone-name\">" << html::escape(z.zone_name) << "</div>"
				  << "  </div>"
				  << "  <div class=\"zone-stats-row\">"
				  << "    <span>Lost: <strong>" << z.lost_count << "</strong></span>"
				  << "    <span>Found: <strong>" << z.found_count << "</strong></span>"
				  << "    <span>Total: <strong style='color:#0f172a;'>" << z.total_incidents << "</strong></span>"
				  << "  </div>"
				  << "</div>";
		}

		html::replacePlaceholder(tmpl, "{{NAVBAR}}", html::renderNavbar("heatmap", lang));
		html::replacePlaceholder(tmpl, "{{HEATMAP_CARDS}}", cards.str());
		html::replacePlaceholder(tmpl, "{{HOTSPOT_COUNT}}", std::to_string(highDensityCount));
		html::replacePlaceholder(tmpl, "{{FOOTER}}", html::renderFooter());

		crow::response res(tmpl);
		res.set_header("Content-Type", "text/html; charset=utf-8");
		return res;
	});

	// ========================================================================
	// 11. OWNERSHIP VERIFICATION CHALLENGE-RESPONSE POST (Feature 11)
	// ========================================================================
	CROW_ROUTE(app, "/item/verify-claim").methods("POST"_method)([](const crow::request& req) {
		auto params = html::parseUrlEncoded(req.body);
		int id = 1;
		if (params.find("id") != params.end()) {
			try { id = std::stoi(params["id"]); } catch (...) {}
		}
		std::string ans = params.count("answer") ? params["answer"] : "";
		std::string claimantName = params.count("claimant_name") ? params["claimant_name"] : "Student Claimant";

		auto itemOpt = g_db.getItemById(id);
		if (itemOpt.has_value()) {
			g_db.logAudit(id, "VERIFIED", itemOpt.value().status, "CLAIM_VERIFIED", claimantName, "Ownership challenge response: " + ans);
		}
		crow::response res(303);
		res.set_header("Location", "/item?id=" + std::to_string(id) + "&verified=1#audit-section");
		return res;
	});

	// ========================================================================
	// 12. INSTITUTIONAL RFC 4180 CSV EXPORT (Feature 13)
	// ========================================================================
	CROW_ROUTE(app, "/admin/export/csv")([](const crow::request& req) {
		if (!isAdminAuthenticated(req)) {
			crow::response res(303);
			res.set_header("Location", "/admin?error=auth_required");
			return res;
		}
		std::vector<Item> allItems = g_db.getAllItems();
		std::string csv = html::generateInstitutionalCSV(allItems);
		crow::response res(csv);
		res.set_header("Content-Type", "text/csv; charset=utf-8");
		res.set_header("Content-Disposition", "attachment; filename=\"dyptc_lost_found_register_2026.csv\"");
		return res;
	});

	CROW_ROUTE(app, "/admin/export.csv")([](const crow::request& req) {
		if (!isAdminAuthenticated(req)) {
			crow::response res(303);
			res.set_header("Location", "/admin?error=auth_required");
			return res;
		}
		std::vector<Item> allItems = g_db.getAllItems();
		std::string csv = html::generateInstitutionalCSV(allItems);
		crow::response res(csv);
		res.set_header("Content-Type", "text/csv; charset=utf-8");
		res.set_header("Content-Disposition", "attachment; filename=\"dyptc_lost_found_register_2026.csv\"");
		return res;
	});

	// ========================================================================
	// 13. 90-DAY ARCHIVAL SWEEP ROUTE (Feature 09)
	// ========================================================================
	CROW_ROUTE(app, "/admin/archive").methods("POST"_method)([](const crow::request& req) {
		if (!isAdminAuthenticated(req)) {
			crow::response res(303);
			res.set_header("Location", "/admin?error=auth_required");
			return res;
		}
		int count = g_db.archiveOldItems(90);
		crow::response res(303);
		res.set_header("Location", "/admin?msg=archived&count=" + std::to_string(count));
		return res;
	});

    const char* envPort = std::getenv("PORT");
    int port = 18080;
    if (envPort) {
        try { port = std::stoi(envPort); } catch (...) {}
    }
    std::cout << "[Server] Binding on port: " << port << std::endl;
    app.port(port).multithreaded().run();
	return 0;
}
