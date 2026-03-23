(function () {
  const current = window.location.pathname.replace(/\\/g, "/").toLowerCase();
  document.querySelectorAll("[data-nav]").forEach((link) => {
    const target = (link.getAttribute("href") || "").toLowerCase();
    if (!target) return;

    if (current.endsWith(target.replace("./", "")) || (target === "../index.html" && /docssite\/?$/.test(current))) {
      link.classList.add("active");
    }
  });
})();
