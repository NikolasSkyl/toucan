(function () {
  function escapeHtml(text) {
    return text
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;');
  }

  function highlightDonna(source) {
    let html = escapeHtml(source);
    const held = {};
    let heldCount = 0;

    function holdKey(index) {
      let out = '';
      let n = index;
      do {
        out = String.fromCharCode(65 + (n % 26)) + out;
        n = Math.floor(n / 26) - 1;
      } while (n >= 0);
      return `@@DONNAHL${out}@@`;
    }

    function stash(cls, value) {
      const key = holdKey(heldCount);
      heldCount += 1;
      held[key] = `<span class="${cls}">${value}</span>`;
      return key;
    }

    html = html.replace(/(&quot;[^\n]*?&quot;|"[^\n]*?")/g, (match) => stash('hl-str', match));
    html = html.replace(/(\/\/.*)$/gm, (match) => stash('hl-comment', match));
    html = html.replace(/(\bimport\s+)([A-Za-z0-9_\/]+)/g, (_match, _kw, mod) => `${stash('hl-kw', 'import')} ${stash('hl-module', mod)}`);
    html = html.replace(/\b([a-z_][A-Za-z0-9_]*)(?=\.)/g, (match) => stash('hl-module', match));
    html = html.replace(/(\|&gt;\s*)([a-z_][A-Za-z0-9_]*)/g, (_match, op, fn) => `${stash('hl-op', op.trim())} ${stash('hl-fn', fn)}`);
    html = html.replace(/(\|&gt;|-&gt;|=&gt;|&lt;&gt;)/g, (match) => stash('hl-op', match));
    html = html.replace(/\blet\s+([a-z_][A-Za-z0-9_]*)/g, (_match, name) => `${stash('hl-kw', 'let')} ${stash('hl-var', name)}`);
    html = html.replace(/\b(echo|panic)\s+([a-z_][A-Za-z0-9_]*)\b/g, (_match, kw, name) => `${stash('hl-kw', kw)} ${stash('hl-var', name)}`);
    html = html.replace(/\bcase\s+([a-z_][A-Za-z0-9_]*)\b(?!\s*\()/g, (_match, name) => `${stash('hl-kw', 'case')} ${stash('hl-var', name)}`);
    html = html.replace(/\b(pub|fn|type|const|case|if|opaque|external|echo|panic|todo|as)\b/g, (match) => stash('hl-kw', match));
    html = html.replace(/\b([a-z_][A-Za-z0-9_]*)(?=\s*\()/g, (match) => stash('hl-fn', match));
    html = html.replace(/\b([A-Z][A-Za-z0-9_]*)(?=\s*\()/g, (match) => stash('hl-constructor', match));
    html = html.replace(/\b(Int|String|Bool|Float|List|Nil|Result|Option)\b/g, (match) => stash('hl-type', match));
    html = html.replace(/\b(True|False|Some|None|Ok|Err)\b/g, (match) => stash('hl-constructor', match));
    html = html.replace(/\b([0-9]+(?:\.[0-9]+)?)\b/g, (match) => stash('hl-num', match));
    html = html.replace(/@@DONNAHL[A-Z]+@@/g, (match) => held[match] || match);
    return html;
  }

  function initDonnaHighlight() {
    document.querySelectorAll('pre code').forEach((code) => {
      if (code.dataset.highlighted) return;
      if (!/\b(pub fn|fn |import |pub type|pub const|case |let )\b/.test(code.textContent || '')) return;
      code.innerHTML = highlightDonna(code.textContent || '');
      code.dataset.highlighted = 'true';
    });
  }

  var navBtn = document.getElementById('nav-toggle');
  var nav = document.getElementById('site-nav');
  if (navBtn && nav) {
    navBtn.addEventListener('click', function () {
      var open = nav.classList.toggle('open');
      navBtn.setAttribute('aria-expanded', String(open));
    });
  }

  var sidebarBtn = document.getElementById('sidebar-toggle');
  var sidebar = document.getElementById('docs-sidebar');
  var overlay = document.getElementById('sidebar-overlay');

  function closeSidebar() {
    if (!sidebar || !overlay || !sidebarBtn) return;
    sidebar.classList.remove('open');
    overlay.classList.remove('open');
    sidebarBtn.setAttribute('aria-expanded', 'false');
  }

  if (sidebarBtn && sidebar && overlay) {
    sidebarBtn.addEventListener('click', function () {
      var open = sidebar.classList.toggle('open');
      overlay.classList.toggle('open', open);
      sidebarBtn.setAttribute('aria-expanded', String(open));
    });

    overlay.addEventListener('click', closeSidebar);
    sidebar.addEventListener('click', function (event) {
      if (event.target && event.target.tagName === 'A') closeSidebar();
    });
  }
  initDonnaHighlight();
}());
