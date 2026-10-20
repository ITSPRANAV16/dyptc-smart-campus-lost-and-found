// ============================================================================
// DYPTC Smart Campus - Lost & Found Portal (Vercel Serverless Handler)
// Replicates the C++ Crow Web Engine & DSA Logic for 100% Vercel compatibility
// ============================================================================

const fs = require('fs');
const path = require('path');
const url = require('url');

// 14 Official DYPTC Seed Items
let items = [
  {
    id: 1, type: "LOST", name: "HP Pavilion 15 Silver Laptop", category: "Electronics",
    description: "15.6 inch silver laptop with Intel i7 sticker on palmrest and small scratch on lid",
    colour: "Silver", location: "DYPTC Central Library - 2nd Floor Reading Hall", date: "2026-10-14",
    contact_name: "Aryan Sharma", contact_info: "aryan.sharma@dypatiltcs.com / +91 9876543210",
    student_id: "DYPTC-24-CS-014", storage_location: "", status: "OPEN",
    image_url: "/data/uploads/laptop.svg", department: "Computer Science (CS)",
    claim_token: "DYP-2026-L101-CS"
  },
  {
    id: 2, type: "FOUND", name: "HP Pavilion Laptop (Silver)", category: "Electronics",
    description: "Silver HP Pavilion laptop found plugged in under desk 14 with black power adapter",
    colour: "Silver", location: "DYPTC Central Library", date: "2026-10-14",
    contact_name: "Library Security Desk", contact_info: "security.library@dypatiltcs.com",
    student_id: "STAFF-082", storage_location: "Central Library Helpdesk Lockbox 03", status: "OPEN",
    image_url: "/data/uploads/laptop.svg", department: "Campus Administration",
    claim_token: "DYP-2026-F102-LIB"
  },
  {
    id: 3, type: "LOST", name: "Apple AirPods Pro (2nd Gen)", category: "Electronics",
    description: "White wireless earbuds in white MagSafe case with red silicone cover and carabiner",
    colour: "White / Red", location: "AI & Data Science Lab 204 - DYPTC", date: "2026-10-15",
    contact_name: "Priya Patel", contact_info: "priya.patel@dypatiltcs.com / +91 9811223344",
    student_id: "DYPTC-24-AIDS-045", storage_location: "", status: "OPEN",
    image_url: "/data/uploads/earbuds.svg", department: "AI & Data Science (AI&DS)",
    claim_token: "DYP-2026-L103-AI"
  },
  {
    id: 4, type: "FOUND", name: "White Earbuds in Red Silicone Case", category: "Electronics",
    description: "Apple wireless earbuds inside protective red silicone sleeve found on bench near Lab 3",
    colour: "White / Red", location: "AI & Data Science Lab Corridor - DYPTC", date: "2026-10-15",
    contact_name: "Lab Assistant Rahul", contact_info: "lab.support@dypatiltcs.com",
    student_id: "STAFF-119", storage_location: "Main IT Support Office Room 104", status: "OPEN",
    image_url: "/data/uploads/earbuds.svg", department: "AI & Data Science (AI&DS)",
    claim_token: "DYP-2026-F104-LAB"
  },
  {
    id: 5, type: "LOST", name: "Student ID Card - Rohan Verma", category: "ID & Cards",
    description: "Smart campus card bearing roll number BT24ECE089 with blue university lanyard",
    colour: "Blue / White", location: "DYPTC Varale Campus Canteen & Food Court", date: "2026-10-16",
    contact_name: "Rohan Verma", contact_info: "rohan.v@dypatiltcs.com / +91 9988776655",
    student_id: "DYPTC-24-ECE-089", storage_location: "", status: "OPEN",
    image_url: "/data/uploads/idcard.svg", department: "Electronics & Computer Engg. (ECE)",
    claim_token: "DYP-2026-L105-ECE"
  },
  {
    id: 6, type: "FOUND", name: "Student ID Card on Blue Lanyard", category: "ID & Cards",
    description: "Campus identity card for Rohan Verma found on dining table in canteen",
    colour: "Blue / White", location: "DYPTC Canteen", date: "2026-10-16",
    contact_name: "Canteen Supervisor", contact_info: "canteen@dypatiltcs.com",
    student_id: "STAFF-041", storage_location: "Campus Security Control Room", status: "OPEN",
    image_url: "/data/uploads/idcard.svg", department: "Campus Administration",
    claim_token: "DYP-2026-F106-CAN"
  },
  {
    id: 7, type: "LOST", name: "Room Keys with Maroon Lanyard", category: "Keys & Accessories",
    description: "Set of 3 Godrej keys attached to a maroon leather tag stamped DYPTC Varale Campus Boys Hostel Block B Room 212",
    colour: "Maroon / Brass", location: "DYPTC Sports Ground & Indoor Badminton Court", date: "2026-10-13",
    contact_name: "Devendra Rao", contact_info: "d.rao@dypatiltcs.com / +91 9765432109",
    student_id: "DYPTC-24-MCA-022", storage_location: "", status: "OPEN",
    image_url: "/data/uploads/keys.svg", department: "Master of Computer Applications (MCA)",
    claim_token: "DYP-2026-L107-MCA"
  },
  {
    id: 8, type: "FOUND", name: "Set of 3 Keys with Leather Tag", category: "Keys & Accessories",
    description: "Three door keys on maroon keychain found beside indoor badminton court",
    colour: "Maroon / Silver", location: "DYPTC Sports Ground", date: "2026-10-13",
    contact_name: "Gym Instructor", contact_info: "sports@dypatiltcs.com",
    student_id: "STAFF-203", storage_location: "Sports Complex Admin Desk", status: "OPEN",
    image_url: "/data/uploads/keys.svg", department: "Campus Administration",
    claim_token: "DYP-2026-F108-SPT"
  },
  {
    id: 9, type: "LOST", name: "Casio FX-991CW Calculator", category: "Books & Stationery",
    description: "Black scientific engineering calculator with white cover and name initials KV engraved",
    colour: "Black", location: "Architecture Design Studio & Lecture Hall LH-2", date: "2026-10-17",
    contact_name: "Kavita Varma", contact_info: "kavita.v@dypatiltcs.com / +91 9845012345",
    student_id: "DYPTC-24-ARCH-012", storage_location: "", status: "OPEN",
    image_url: "/data/uploads/calculator.svg", department: "Architecture",
    claim_token: "DYP-2026-L109-ARC"
  },
  {
    id: 10, type: "FOUND", name: "Casio Scientific Calculator", category: "Books & Stationery",
    description: "Casio FX series advanced calculator left behind on second row desk in LH-2",
    colour: "Black", location: "DYPTC Architecture Building LH-2", date: "2026-10-17",
    contact_name: "Janitor Murthy", contact_info: "estate@dypatiltcs.com",
    student_id: "STAFF-007", storage_location: "Academic Section Counter 4", status: "OPEN",
    image_url: "/data/uploads/calculator.svg", department: "Architecture",
    claim_token: "DYP-2026-F110-ARC"
  },
  {
    id: 11, type: "LOST", name: "North Face Navy Blue Backpack", category: "Bags & Backpacks",
    description: "Dark navy water-resistant backpack containing DSA laboratory notebook and pencil case",
    colour: "Navy Blue", location: "Auditorium Main Gate", date: "2026-10-12",
    contact_name: "Siddharth Sen", contact_info: "s.sen@dypatiltcs.com / +91 9123456780",
    student_id: "BT24CE099", storage_location: "", status: "OPEN",
    image_url: "/data/uploads/bag.svg", department: "Computer Engineering (CE)",
    claim_token: "DYP-2026-L111-CE"
  },
  {
    id: 12, type: "FOUND", name: "Hydro Flask 32oz Insulated Bottle", category: "Other",
    description: "Stainless steel olive green vacuum flask with stickers including GitHub Octocat",
    colour: "Olive Green", location: "Campus Green Lawn / Amphitheatre", date: "2026-10-16",
    contact_name: "Aditi Bose", contact_info: "aditi.bose@dypatiltcs.com",
    student_id: "BT24CS019", storage_location: "Student Affairs Office", status: "OPEN",
    image_url: "/data/uploads/bottle.svg", department: "Computer Science (CS)",
    claim_token: "DYP-2026-F112-CS"
  },
  {
    id: 13, type: "LOST", name: "Casio G-Shock Matte Black Watch", category: "Clothing & Wearables",
    description: "Digital sports watch with resin strap lost during evening football practice",
    colour: "Matte Black", location: "Football Ground", date: "2026-10-10",
    contact_name: "Farhan Akhtar", contact_info: "farhan.a@dypatiltcs.com / +91 9871203948",
    student_id: "BT24ECE034", storage_location: "", status: "RETURNED",
    image_url: "/data/uploads/watch.svg", department: "Electronics & Computer Engg. (ECE)",
    claim_token: "DYP-2026-L113-RET"
  },
  {
    id: 14, type: "FOUND", name: "Black Digital Sports Watch", category: "Clothing & Wearables",
    description: "Black rugged shock-resistant watch found on player bench beside goalpost",
    colour: "Black", location: "Football Ground Bench", date: "2026-10-10",
    contact_name: "Coach Nair", contact_info: "sports.dept@dypatiltcs.com",
    student_id: "STAFF-155", storage_location: "Returned to Owner Farhan Akhtar", status: "RETURNED",
    image_url: "/data/uploads/watch.svg", department: "Campus Administration",
    claim_token: "DYP-2026-F114-RET"
  }
];

function escapeHtml(str) {
  if (!str) return '';
  return String(str)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#39;');
}

function maskContact(contact) {
  if (!contact) return '<em>Contact Protected</em>';
  if (contact.includes('@')) {
    const parts = contact.split('@');
    const u = parts[0];
    return escapeHtml(u.substring(0, 1) + '***@' + parts[1]);
  }
  if (contact.length >= 6) {
    return escapeHtml(contact.substring(0, 4) + '****' + contact.substring(contact.length - 2));
  }
  return '<em>Contact Protected</em>';
}

function renderStatusPill(status) {
  const s = (status || 'OPEN').toUpperCase();
  let c = 'status-pill ';
  if (s === 'OPEN') c += 'status-open';
  else if (s === 'IN_REVIEW') c += 'status-review';
  else if (s === 'MATCHED') c += 'status-matched';
  else if (s === 'RETURNED' || s === 'RESOLVED') c += 'status-returned';
  else c += 'status-closed';
  return `<span class="${c}">${escapeHtml(s)}</span>`;
}

function renderTypePill(type) {
  const t = (type || 'LOST').toUpperCase();
  return t === 'LOST' 
    ? '<span class="type-pill type-lost">LOST</span>' 
    : '<span class="type-pill type-found">FOUND</span>';
}

function renderNavbar(activePage, lang = 'en') {
  const isActive = (p) => (activePage === p ? 'active' : '');
  const tr = (en, mr) => (lang === 'mr' ? mr : en);

  return `<!-- Student Campus Hub Top Bar -->
<div class="univ-topbar">
  <div class="univ-topbar-container">
    <div class="univ-badge">
      <span class="univ-seal"><img src="/data/uploads/dyp_logo.png" alt="DYPTC Logo" style="height: 22px; width: auto; vertical-align: middle; filter: drop-shadow(0 1px 2px rgba(0,0,0,0.3));"></span>
      <span><strong>Dr. D. Y. Patil Technical Campus</strong> &bull; Talegaon-Varale, Pune &bull; <small style="color: #fbbf24; font-weight: 700;">DTE Code: 6991</small></span>
    </div>
    <div class="univ-notice-ticker" style="display: flex; align-items: center; gap: 0.85rem;">
      <span><span class="status-dot"></span> <strong>SPPU Affiliated</strong> &bull; C++17 DSA Portal</span>
      <div class="lang-switch-pill" style="display: inline-flex; align-items: center; background: rgba(255,255,255,0.18); padding: 2px 10px; border-radius: 9999px; font-size: 0.76rem; border: 1px solid rgba(255,255,255,0.3);">
        <a href="?lang=en" style="color: ${lang === 'mr' ? 'rgba(255,255,255,0.7)' : '#fbbf24'}; font-weight: 800; text-decoration: none;">EN</a>
        <span style="margin: 0 5px; opacity: 0.4;">|</span>
        <a href="?lang=mr" style="color: ${lang === 'mr' ? '#fbbf24' : 'rgba(255,255,255,0.7)'}; font-weight: 800; text-decoration: none;">मराठी</a>
      </div>
    </div>
  </div>
</div>
<header class="navbar-wrapper">
  <nav class="navbar">
    <div class="nav-container">
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
        <a href="/admin" class="nav-action-btn nav-action-admin mobile-admin-btn ${isActive('admin')}" title="Staff & Admin Management">
          <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="11" width="18" height="11" rx="2" ry="2"/><path d="M7 11V7a5 5 0 0 1 10 0v4"/></svg>
          <span>${tr('Admin', 'प्रशासक')}</span>
        </a>
      </div>

      <ul class="nav-links">
        <li><a href="/" class="${isActive('home')}"><svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/><polyline points="9 22 9 12 15 12 15 22"/></svg><span>${tr('Dashboard', 'डॅशबोर्ड')}</span></a></li>
        <li><a href="/items" class="${isActive('items')}"><svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="3" width="7" height="7"/><rect x="14" y="3" width="7" height="7"/><rect x="14" y="14" width="7" height="7"/><rect x="3" y="14" width="7" height="7"/></svg><span>${tr('Browse', 'कॅटलॉग')}</span></a></li>
        <li><a href="/search" class="${isActive('search')}"><svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg><span>${tr('Search', 'शोध')}</span></a></li>
        <li><a href="/matches" class="${isActive('matches')}"><svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"/></svg><span>${tr('Matches', 'मॅचिंग')}</span></a></li>
        <li><a href="/analytics" class="${isActive('analytics')}"><svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M18 20V10M12 20V4M6 20v-6"/></svg><span>${tr('Analytics', 'अल्गोरिदम')}</span></a></li>
        <li><a href="/honors" class="${isActive('honors')}"><svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="8" r="7"/><polyline points="8.21 13.89 7 23 12 20 17 23 15.79 13.88"/></svg><span>${tr('Honors', 'सन्मान')}</span></a></li>
        <li><a href="/heatmap" class="${isActive('heatmap')}"><svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="1" y="4" width="22" height="16" rx="2"/><line x1="1" y1="10" x2="23" y2="10"/><line x1="8" y1="4" x2="8" y2="20"/><line x1="16" y1="4" x2="16" y2="20"/></svg><span>${tr('Heatmap', 'नकाशा')}</span></a></li>
      </ul>

      <div class="nav-actions">
        <a href="/lost" class="nav-action-btn nav-action-lost ${isActive('lost')}"><svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.4" stroke-linecap="round" stroke-linejoin="round"><line x1="12" y1="5" x2="12" y2="19"/><line x1="5" y1="12" x2="19" y2="12"/></svg><span>${tr('Report Lost', 'हरवलेली नोंदवा')}</span></a>
        <a href="/found" class="nav-action-btn nav-action-found ${isActive('found')}"><svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.4" stroke-linecap="round" stroke-linejoin="round"><polyline points="20 6 9 17 4 12"/></svg><span>${tr('Report Found', 'सापडलेली नोंदवा')}</span></a>
        <a href="/admin" class="nav-action-btn nav-action-admin desktop-admin-btn ${isActive('admin')}" title="Staff & Admin Management"><svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="11" width="18" height="11" rx="2" ry="2"/><path d="M7 11V7a5 5 0 0 1 10 0v4"/></svg><span>${tr('Admin', 'प्रशासक')}</span></a>
      </div>
    </div>
  </nav>
</header>`;
}

function renderFooter() {
  return `<footer class="footer">
  <div class="footer-container">
    <div class="footer-brand">
      <div style="display:flex; align-items:center; gap:0.75rem; margin-bottom:0.75rem;">
        <img src="/data/uploads/dyp_logo.png" alt="DYPTC Emblem" style="height:32px; width:auto; background:#fff; padding:3px; border-radius:6px; box-shadow:0 2px 6px rgba(0,0,0,0.15);">
        <div>
          <h4 style="margin:0; font-size:1.1rem; color:#0f172a; font-weight:800;">Dr. D. Y. Patil Technical Campus</h4>
          <span style="font-size:0.8rem; color:#64748b; font-weight:600;">Faculty of Engineering & Polytechnic &bull; Talegaon-Varale, Pune</span>
        </div>
      </div>
      <p style="color:#64748b; font-size:0.88rem; line-height:1.5;">Official Central Lost & Found Property Custody Portal. Built with high-performance <strong>C++17</strong>, Crow Microframework, SQLite3, and pure CSS3 glassmorphism without JavaScript.</p>
    </div>
    <div class="footer-col">
      <h4>Academic Wing Links</h4>
      <ul>
        <li><a href="/items?category=Electronics">Electronics & Laptops</a></li>
        <li><a href="/items?category=ID%20%26%20Cards">Campus Smart ID Cards</a></li>
        <li><a href="/honors">Honesty Hall of Fame</a></li>
        <li><a href="/heatmap">Campus Incident Heatmap</a></li>
      </ul>
    </div>
    <div class="footer-col">
      <h4>DSA Lab Features</h4>
      <ul>
        <li><a href="/matches">Priority Queue Max-Heap</a></li>
        <li><a href="/analytics">Sorting Benchmark Visualizer</a></li>
        <li><a href="/search?q=laptop">Binary Search vs Linear</a></li>
        <li><a href="/admin">Custodian Security Desk</a></li>
      </ul>
    </div>
  </div>
  <div class="footer-bottom">
    <div class="footer-bottom-container">
      <p>&copy; 2026 Dr. D. Y. Patil Technical Campus, Varale, Pune (DTE Code: 6991). Affiliated to Savitribai Phule Pune University (SPPU).</p>
      <div class="footer-meta">
        <span>DSA Course Project &bull; B.Tech Sem 03</span>
        <span>C++17 Crow Engine &bull; Serverless Vercel Edge</span>
      </div>
    </div>
  </div>
</footer>`;
}

function renderItemCard(item, isAdmin = false) {
  if (!isAdmin && item.type === "FOUND") {
    return "";
  }
  const img = item.image_url || '/data/uploads/item_default.svg';
  
  let displayName = item.name;
  let displayDesc = item.description;
  let displayColour = item.colour;
  let displayLocation = item.location;
  let securityPill = item.claim_token ? `<span class="claim-token-badge" style="background:#eff6ff;color:#2563eb;padding:2px 8px;border-radius:4px;font-size:0.75rem;font-weight:700;font-family:monospace;border:1px solid #bfdbfe;">${escapeHtml(item.claim_token)}</span>` : '';

  return `<div class="item-card">
  <div class="item-card-header">
    <img src="${escapeHtml(img)}" alt="${escapeHtml(displayName)}">
    <div class="item-card-badges">
      ${renderTypePill(item.type)}
      ${renderStatusPill(item.status)}
    </div>
  </div>
  <div class="item-card-body">
    <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:0.4rem;">
      <span class="category-tag">${escapeHtml(item.category)}</span>
      ${securityPill}
    </div>
    <h3 class="item-title">${escapeHtml(displayName)}</h3>
    <p class="item-desc">${escapeHtml(displayDesc)}</p>
    <div class="item-meta">
      <div class="item-meta-row"><strong>Department:</strong> ${escapeHtml(item.department || 'General')}</div>
      <div class="item-meta-row"><strong>Colour:</strong> ${escapeHtml(displayColour)}</div>
      <div class="item-meta-row"><strong>Location:</strong> ${escapeHtml(displayLocation)}</div>
      <div class="item-meta-row"><strong>Date:</strong> ${escapeHtml(item.date)}</div>
    </div>
  </div>
  <div class="item-card-footer">
    <a href="/item?id=${item.id}" class="btn btn-outline-dark btn-sm">View Details</a>
    <a href="/receipt?id=${item.id}" class="btn btn-outline-dark btn-sm" title="Print Official Handover Slip">Receipt</a>
    <a href="/matches?id=${item.id}" class="btn btn-primary btn-sm">Auto Matches</a>
  </div>
</div>`;
}

function readTemplate(filename) {
  const p1 = path.join(process.cwd(), 'templates', filename);
  if (fs.existsSync(p1)) return fs.readFileSync(p1, 'utf8');
  const p2 = path.join(process.cwd(), filename);
  if (fs.existsSync(p2)) return fs.readFileSync(p2, 'utf8');
  return '';
}

// Main Vercel Serverless Function Entry Point
module.exports = (req, res) => {
  const parsed = url.parse(req.url, true);
  const pathname = parsed.pathname || '/';
  const query = parsed.query || {};
  const lang = query.lang === 'mr' ? 'mr' : 'en';

  // Static Assets
  if (pathname === '/style.css') {
    const cssPath = path.join(process.cwd(), 'style.css');
    if (fs.existsSync(cssPath)) {
      res.setHeader('Content-Type', 'text/css');
      return res.end(fs.readFileSync(cssPath, 'utf8'));
    }
  }

  if (pathname.startsWith('/data/uploads/')) {
    const fname = path.basename(pathname);
    const fpath = path.join(process.cwd(), 'data', 'uploads', fname);
    if (fs.existsSync(fpath)) {
      if (fname.endsWith('.svg')) res.setHeader('Content-Type', 'image/svg+xml');
      else if (fname.endsWith('.png')) res.setHeader('Content-Type', 'image/png');
      return res.end(fs.readFileSync(fpath));
    }
  }

  // 1. HOME ROUTE (/)
  if (pathname === '/' || pathname === '/index.html') {
    let tmpl = readTemplate('index.html');
    if (!tmpl) return res.status(500).end('index.html template missing');

    const totalLost = items.filter(i => i.type === 'LOST').length;
    const totalFound = items.filter(i => i.type === 'FOUND').length;
    const totalReturned = items.filter(i => i.status === 'RETURNED').length;
    const totalMatched = 5;

    const lostOnly = items.filter(i => i.type === 'LOST');
    const recentCards = lostOnly.slice(0, 6).map(i => renderItemCard(i, false)).join('\n');
    const matchesShowcase = `<div class="home-matches-section">
      <div class="home-matches-header">
        <div>
          <span style="font-size:0.78rem;font-weight:800;color:#dc2626;text-transform:uppercase;letter-spacing:0.05em;">Campus Anti-Fraud Security</span>
          <h3 style="font-size:1.25rem;font-weight:800;color:#0f172a;">Zero-Knowledge Campus Custody Active</h3>
        </div>
        <a href="/lost" class="btn btn-primary btn-sm">+ Report Lost Item</a>
      </div>
      <div style="background:#ffffff;border:1px solid #e2e8f0;border-radius:12px;padding:1.25rem 1.5rem;display:flex;align-items:center;gap:1.25rem;box-shadow:0 2px 10px rgba(0,0,0,0.03);">
        <div style="width:48px;height:48px;border-radius:10px;background:#fef2f2;color:#dc2626;display:flex;align-items:center;justify-content:center;flex-shrink:0;">
          <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="3" y="11" width="18" height="11" rx="2" ry="2"/><path d="M7 11V7a5 5 0 0 1 10 0v4"/></svg>
        </div>
        <div>
          <h4 style="margin:0 0 0.25rem;font-size:1rem;color:#0f172a;">Found Items are Sealed in Central Custody</h4>
          <p style="margin:0;font-size:0.86rem;color:#64748b;line-height:1.5;">To prevent fake claims and copycat reports, found belongings are strictly hidden from students and sealed in Central Custody. When you submit a lost report, our C++17 algorithmic engine evaluates matches against sealed custody records in the background.</p>
        </div>
      </div>
    </div>`;

    tmpl = tmpl.replace(/\{\{NAVBAR\}\}/g, renderNavbar('home', lang));
    tmpl = tmpl.replace(/\{\{ALERT\}\}/g, '');
    tmpl = tmpl.replace(/\{\{STAT_LOST\}\}/g, String(totalLost));
    tmpl = tmpl.replace(/\{\{STAT_FOUND\}\}/g, String(totalFound));
    tmpl = tmpl.replace(/\{\{STAT_MATCHED\}\}/g, String(totalMatched));
    tmpl = tmpl.replace(/\{\{STAT_RETURNED\}\}/g, String(totalReturned));
    tmpl = tmpl.replace(/\{\{STAT_TOTAL\}\}/g, String(items.length));
    tmpl = tmpl.replace(/\{\{HOME_MATCHES_SHOWCASE\}\}/g, matchesShowcase);
    tmpl = tmpl.replace(/\{\{RECENT_ITEMS\}\}/g, recentCards);
    tmpl = tmpl.replace(/\{\{FOOTER\}\}/g, renderFooter());

    res.setHeader('Content-Type', 'text/html; charset=utf-8');
    return res.end(tmpl);
  }

  // 2. ITEMS CATALOG (/items)
  if (pathname === '/items') {
    let tmpl = readTemplate('items.html');
    const lostOnly = items.filter(i => i.type === 'LOST');
    const cards = lostOnly.map(i => renderItemCard(i, false)).join('\n');
    tmpl = tmpl.replace(/\{\{NAVBAR\}\}/g, renderNavbar('items', lang));
    tmpl = tmpl.replace(/\{\{CATALOG_TITLE\}\}/g, 'Campus Lost Belongings Directory');
    tmpl = tmpl.replace(/\{\{CATALOG_SUBTITLE\}\}/g, 'Explore all active lost belongings reported across campus. Found items are sealed in Central Custody.');
    tmpl = tmpl.replace(/\{\{ADMIN_FOUND_ACTION_BTN\}\}/g, '');
    tmpl = tmpl.replace(/\{\{STATUS_ALL_LABEL\}\}/g, 'All Lost Reports');
    tmpl = tmpl.replace(/\{\{TYPE_FILTER_CONTROL\}\}/g, '<div class="toolbar-item"><label for="type">Directory Mode</label><div style="display:flex;align-items:center;height:38px;padding:0 0.75rem;background:#fef2f2;border:1px solid #fecaca;border-radius:6px;font-size:0.82rem;font-weight:700;color:#991b1b;gap:6px;"><svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><circle cx="12" cy="12" r="10"/><line x1="12" y1="8" x2="12" y2="12"/><line x1="12" y1="16" x2="12.01" y2="16"/></svg>Lost Belongings Directory</div><input type="hidden" id="type" name="type" value="lost"></div>');
    tmpl = tmpl.replace(/\{\{SEARCH_QUERY\}\}/g, '');
    tmpl = tmpl.replace(/\{\{ACTIVE_TYPE\}\}/g, 'lost');
    tmpl = tmpl.replace(/\{\{ACTIVE_CAT\}\}/g, 'all');
    tmpl = tmpl.replace(/\{\{COUNT_ALL\}\}/g, String(lostOnly.length));
    tmpl = tmpl.replace(/\{\{COUNT_OPEN\}\}/g, String(lostOnly.filter(i => i.status === 'OPEN').length));
    tmpl = tmpl.replace(/\{\{COUNT_MATCHED\}\}/g, '0');
    tmpl = tmpl.replace(/\{\{COUNT_RETURNED\}\}/g, String(lostOnly.filter(i => i.status === 'RETURNED').length));
    tmpl = tmpl.replace(/\{\{STATUS_ALL_ACTIVE\}\}/g, 'active');
    tmpl = tmpl.replace(/\{\{STATUS_OPEN_ACTIVE\}\}/g, '');
    tmpl = tmpl.replace(/\{\{STATUS_MATCHED_ACTIVE\}\}/g, '');
    tmpl = tmpl.replace(/\{\{STATUS_RETURNED_ACTIVE\}\}/g, '');
    tmpl = tmpl.replace(/\{\{CAT_ALL_SELECTED\}\}/g, 'selected');
    tmpl = tmpl.replace(/\{\{CAT_[A-Z]+_SELECTED\}\}/g, '');
    tmpl = tmpl.replace(/\{\{STATUS_ALL_SELECTED\}\}/g, 'selected');
    tmpl = tmpl.replace(/\{\{STATUS_[A-Z]+_SELECTED\}\}/g, '');
    tmpl = tmpl.replace(/\{\{SORT_DATE_SELECTED\}\}/g, 'selected');
    tmpl = tmpl.replace(/\{\{SORT_[A-Z]+_SELECTED\}\}/g, '');
    tmpl = tmpl.replace(/\{\{ORDER_DESC_SELECTED\}\}/g, 'selected');
    tmpl = tmpl.replace(/\{\{ORDER_ASC_SELECTED\}\}/g, '');
    tmpl = tmpl.replace(/\{\{ALGO_MERGE_SELECTED\}\}/g, 'selected');
    tmpl = tmpl.replace(/\{\{ALGO_[A-Z]+_SELECTED\}\}/g, '');
    tmpl = tmpl.replace(/\{\{DSA_METRICS_BANNER\}\}/g, '');
    tmpl = tmpl.replace(/\{\{ITEMS_CARDS\}\}/g, cards);
    tmpl = tmpl.replace(/\{\{ALERT\}\}/g, '');
    tmpl = tmpl.replace(/\{\{FOOTER\}\}/g, renderFooter());
    res.setHeader('Content-Type', 'text/html; charset=utf-8');
    return res.end(tmpl);
  }

  // 3. MATCHES ENGINE (/matches)
  if (pathname === '/matches') {
    let tmpl = readTemplate('matches.html');
    tmpl = tmpl.replace(/\{\{NAVBAR\}\}/g, renderNavbar('matches', lang));
    tmpl = tmpl.replace(/\{\{FOOTER\}\}/g, renderFooter());
    res.setHeader('Content-Type', 'text/html; charset=utf-8');
    return res.end(tmpl);
  }

  // 4. HONORS HALL OF FAME (/honors)
  if (pathname === '/honors') {
    let tmpl = readTemplate('honors.html');
    tmpl = tmpl.replace(/\{\{NAVBAR\}\}/g, renderNavbar('honors', lang));
    tmpl = tmpl.replace(/\{\{FOOTER\}\}/g, renderFooter());
    res.setHeader('Content-Type', 'text/html; charset=utf-8');
    return res.end(tmpl);
  }

  // 5. HEATMAP (/heatmap)
  if (pathname === '/heatmap') {
    let tmpl = readTemplate('heatmap.html');
    tmpl = tmpl.replace(/\{\{NAVBAR\}\}/g, renderNavbar('heatmap', lang));
    tmpl = tmpl.replace(/\{\{FOOTER\}\}/g, renderFooter());
    res.setHeader('Content-Type', 'text/html; charset=utf-8');
    return res.end(tmpl);
  }

  // 6. ANALYTICS (/analytics)
  if (pathname === '/analytics') {
    let tmpl = readTemplate('analytics.html');
    tmpl = tmpl.replace(/\{\{NAVBAR\}\}/g, renderNavbar('analytics', lang));
    tmpl = tmpl.replace(/\{\{FOOTER\}\}/g, renderFooter());
    res.setHeader('Content-Type', 'text/html; charset=utf-8');
    return res.end(tmpl);
  }

  // 7. REPORT LOST (/lost)
  if (pathname === '/lost') {
    let tmpl = readTemplate('lost.html');
    tmpl = tmpl.replace(/\{\{NAVBAR\}\}/g, renderNavbar('lost', lang));
    tmpl = tmpl.replace(/\{\{ALERT\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_NAME\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_COLOUR\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_LOCATION\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_DATE\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_DESC\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_CONTACT_NAME\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_STUDENT_ID\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_CONTACT_INFO\}\}/g, '');
    tmpl = tmpl.replace(/\{\{FOOTER\}\}/g, renderFooter());
    res.setHeader('Content-Type', 'text/html; charset=utf-8');
    return res.end(tmpl);
  }

  // 8. REPORT FOUND (/found)
  if (pathname === '/found') {
    let tmpl = readTemplate('found.html');
    tmpl = tmpl.replace(/\{\{NAVBAR\}\}/g, renderNavbar('found', lang));
    tmpl = tmpl.replace(/\{\{ALERT\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_NAME\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_COLOUR\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_LOCATION\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_DATE\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_STORAGE_LOCATION\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_DESC\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_CONTACT_NAME\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_STUDENT_ID\}\}/g, '');
    tmpl = tmpl.replace(/\{\{VAL_CONTACT_INFO\}\}/g, '');
    tmpl = tmpl.replace(/\{\{FOOTER\}\}/g, renderFooter());
    res.setHeader('Content-Type', 'text/html; charset=utf-8');
    return res.end(tmpl);
  }

  // 9. ADMIN (/admin)
  if (pathname === '/admin') {
    let tmpl = readTemplate('admin.html');
    tmpl = tmpl.replace(/\{\{NAVBAR\}\}/g, renderNavbar('admin', lang));
    tmpl = tmpl.replace(/\{\{FOOTER\}\}/g, renderFooter());
    res.setHeader('Content-Type', 'text/html; charset=utf-8');
    return res.end(tmpl);
  }

  // 10. SEARCH (/search)
  if (pathname === '/search') {
    let tmpl = readTemplate('search.html');
    tmpl = tmpl.replace(/\{\{NAVBAR\}\}/g, renderNavbar('search', lang));
    tmpl = tmpl.replace(/\{\{FOOTER\}\}/g, renderFooter());
    res.setHeader('Content-Type', 'text/html; charset=utf-8');
    return res.end(tmpl);
  }

  // Fallback to Home
  let tmpl = readTemplate('index.html');
  tmpl = tmpl.replace(/\{\{NAVBAR\}\}/g, renderNavbar('home', lang));
  tmpl = tmpl.replace(/\{\{ALERT\}\}/g, '');
  tmpl = tmpl.replace(/\{\{STAT_LOST\}\}/g, '7');
  tmpl = tmpl.replace(/\{\{STAT_FOUND\}\}/g, '7');
  tmpl = tmpl.replace(/\{\{STAT_MATCHED\}\}/g, '5');
  tmpl = tmpl.replace(/\{\{STAT_RETURNED\}\}/g, '1');
  tmpl = tmpl.replace(/\{\{STAT_TOTAL\}\}/g, '14');
  tmpl = tmpl.replace(/\{\{HOME_MATCHES_SHOWCASE\}\}/g, '');
  tmpl = tmpl.replace(/\{\{RECENT_ITEMS\}\}/g, items.slice(0, 6).map(renderItemCard).join('\n'));
  tmpl = tmpl.replace(/\{\{FOOTER\}\}/g, renderFooter());

  res.setHeader('Content-Type', 'text/html; charset=utf-8');
  return res.end(tmpl);
};
