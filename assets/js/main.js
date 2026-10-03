/**
 * WindowsMovieMakerDecomp - Main JS for primary Jekyll page
 */
document.addEventListener('DOMContentLoaded', () => {
  // Mobile Nav Toggle
  const navToggle = document.getElementById('nav-toggle');
  const siteNav = document.getElementById('site-nav');

  if (navToggle && siteNav) {
    navToggle.addEventListener('click', () => {
      const isOpen = siteNav.classList.toggle('open');
      navToggle.setAttribute('aria-expanded', isOpen ? 'true' : 'false');
    });
  }

  // Copy build commands button
  const copyBtn = document.getElementById('copy-build-btn');
  const snippet = document.getElementById('build-snippet');

  if (copyBtn && snippet) {
    copyBtn.addEventListener('click', async () => {
      try {
        await navigator.clipboard.writeText(snippet.textContent);
        const originalText = copyBtn.innerHTML;
        copyBtn.innerHTML = `
          <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round">
            <polyline points="20 6 9 17 4 12"></polyline>
          </svg>
          <span>Copied!</span>
        `;
        copyBtn.style.borderColor = 'rgba(34, 197, 94, 0.4)';
        copyBtn.style.color = '#4ade80';

        setTimeout(() => {
          copyBtn.innerHTML = originalText;
          copyBtn.style.borderColor = '';
          copyBtn.style.color = '';
        }, 2000);
      } catch (err) {
        console.error('Failed to copy text: ', err);
      }
    });
  }
});
