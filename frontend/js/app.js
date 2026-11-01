// LifeLink shared frontend logic (talks to the C++ Crow REST API)
const API = "/api";
const BLOOD_GROUPS = ["A+","A-","B+","B-","AB+","AB-","O+","O-"];

// ---------- layout ----------
function renderLayout() {
  const page = location.pathname.split("/").pop() || "index.html";
  const links = [
    ["index.html","Home"],["register.html","Become a Donor"],["find.html","Find a Donor"],
    ["request.html","Emergency Request"],["dashboard.html","Dashboard"]
  ];
  document.body.insertAdjacentHTML("afterbegin", `
    <header><nav class="container">
      <a href="index.html" class="logo">🩸 Life<span>Link</span></a>
      <button class="menu-btn" aria-label="Menu" onclick="document.querySelector('.nav-links').classList.toggle('open')">☰</button>
      <ul class="nav-links">${links.map(([h,t]) => `<li><a href="${h}" class="${h===page?"active":""}">${t}</a></li>`).join("")}</ul>
    </nav></header>`);
  document.body.insertAdjacentHTML("beforeend", `
    <footer><div class="container"><p><b>LifeLink</b> – Every Drop Can Save a Life</p>
    <p class="muted" style="color:#aaa">Built with HTML/CSS/JS · C++ (Crow) · SQLite</p></div></footer>
    <div id="toast"></div>`);
  document.querySelectorAll("select[data-blood]").forEach(s => {
    s.insertAdjacentHTML("beforeend", BLOOD_GROUPS.map(g => `<option value="${g}">${g}</option>`).join(""));
  });
  const io = new IntersectionObserver(es => es.forEach(e => e.isIntersecting && e.target.classList.add("visible")), {threshold:.15});
  document.querySelectorAll(".reveal").forEach(el => io.observe(el));
}

// ---------- helpers ----------
function toast(msg, type = "") {
  const t = document.getElementById("toast");
  t.textContent = msg; t.className = "show " + type;
  clearTimeout(t._h); t._h = setTimeout(() => t.className = "", 3500);
}
function esc(s) {
  return String(s ?? "").replace(/[&<>"']/g, c => ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
}
async function api(path, options = {}) {
  let res;
  try {
    res = await fetch(API + path, { headers: {"Content-Type":"application/json"}, ...options });
  } catch {
    throw new Error("Cannot reach the server. Is the C++ backend running?");
  }
  const data = await res.json().catch(() => ({}));
  if (!res.ok) throw new Error(data.error || `Request failed (${res.status})`);
  return data;
}
function animateCount(el, to) {
  let n = 0; const step = Math.max(1, Math.ceil(to / 40));
  const id = setInterval(() => { n = Math.min(to, n + step); el.textContent = n; if (n >= to) clearInterval(id); }, 25);
}

// ---------- validation ----------
const rules = {
  name: v => /^[A-Za-z .'-]{2,100}$/.test(v) || "Use letters only (min 2 characters).",
  phone: v => /^[6-9]\d{9}$/.test(v) || "Enter a valid 10-digit mobile number.",
  email: v => /^[^@\s]+@[^@\s]+\.[^@\s]+$/.test(v) || "Enter a valid email address.",
  age: v => (v >= 18 && v <= 65) || "Age must be between 18 and 65.",
  required: v => v.trim() !== "" || "This field is required.",
  units: v => (v >= 1 && v <= 10) || "Between 1 and 10 units.",
  future: v => (v && v >= new Date().toISOString().slice(0,10)) || "Choose today or a future date.",
};
function validateForm(form) {
  let ok = true;
  form.querySelectorAll("[data-rule]").forEach(input => {
    const field = input.closest(".field");
    const err = field.querySelector(".err");
    const val = input.type === "number" ? Number(input.value) : input.value.trim();
    let msg = "";
    for (const r of input.dataset.rule.split(" ")) {
      const res = rules[r](input.type === "number" && r === "required" ? input.value : val);
      if (res !== true) { msg = res; break; }
    }
    field.classList.toggle("invalid", !!msg);
    if (err) err.textContent = msg;
    if (msg) ok = false;
  });
  return ok;
}
function formData(form) {
  const o = {};
  new FormData(form).forEach((v, k) => o[k] = typeof v === "string" ? v.trim() : v);
  form.querySelectorAll("input[type=checkbox]").forEach(c => o[c.name] = c.checked ? 1 : 0);
  form.querySelectorAll("input[type=number]").forEach(n => o[n.name] = Number(n.value));
  return o;
}

document.addEventListener("DOMContentLoaded", renderLayout);
