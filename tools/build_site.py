#!/usr/bin/env python3
"""
tools/build_site.py

Helper script to assemble and preview the Jekyll primary webpage + Docusaurus /docs
locally or in environments without Ruby/Jekyll.
"""

import os
import re
import shutil
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SITE_DIR = REPO_ROOT / "_site"
WEBSITE_DIR = REPO_ROOT / "website"
WEBSITE_BUILD_DIR = WEBSITE_DIR / "build"


def render_liquid_simple(template_str: str, context: dict) -> str:
    """Simple Liquid variable & include substitution for local offline testing."""
    # Process includes: {% include head.html %}, etc.
    def include_replacer(match):
        inc_name = match.group(1).strip()
        inc_path = REPO_ROOT / "_includes" / inc_name
        if inc_path.exists():
            return render_liquid_simple(inc_path.read_text(encoding="utf-8"), context)
        return ""

    content = re.sub(r"{%\s*include\s+([a-zA-Z0-9_\-\.]+)\s*%}", include_replacer, template_str)

    # Process variables: {{ site.baseurl }}, etc.
    for key, val in context.items():
        pattern = re.compile(r"{{\s*" + re.escape(key) + r"\s*}}")
        content = pattern.sub(str(val), content)

    # Clean any remaining simple conditionals or empty liquid tags if present
    content = re.sub(r"{%\s*if\s+[^%]+\s*%}(.*?){%\s*endif\s*%}", r"\1", content, flags=re.DOTALL)
    content = re.sub(r"{%\s*[^%]+\s*%}", "", content)
    return content


def build_jekyll_offline(baseurl: str = ""):
    """Renders Jekyll site without needing ruby/bundler installed."""
    print(f"[1/3] Building primary Jekyll site (baseurl='{baseurl}')...")
    SITE_DIR.mkdir(parents=True, exist_ok=True)

    context = {
        "site.title": "WindowsMovieMakerDecomp",
        "site.description": "Windows Live Movie Maker 2012 (codename Sundance) reconstructed from binary analysis as C++14 / ATL / WTL source, matching MSVC 11.0 binaries with 116 verified contract tests.",
        "site.url": "https://havaianasdestruido.github.io",
        "site.baseurl": baseurl,
        "site.repository_url": "https://github.com/havaianasdestruido/WindowsMovieMakerDecomp",
        "page.title": "Windows Live Movie Maker 2012 Source Reconstruction",
        "page.description": "Windows Live Movie Maker 2012 (codename Sundance) reconstructed from binary analysis as C++14 / ATL / WTL source, matching MSVC 11.0 binaries with 116 verified contract tests.",
        "page.url": "/",
    }

    # Copy assets
    dest_assets = SITE_DIR / "assets"
    if dest_assets.exists():
        shutil.rmtree(dest_assets)
    shutil.copytree(REPO_ROOT / "assets", dest_assets)

    # Render index.html
    raw_index = (REPO_ROOT / "index.html").read_text(encoding="utf-8")
    # strip frontmatter
    if raw_index.startswith("---"):
        _, _, body = raw_index.split("---", 2)
    else:
        body = raw_index

    # Read default layout
    layout = (REPO_ROOT / "_layouts" / "default.html").read_text(encoding="utf-8")
    page_html = layout.replace("{{ content }}", body)
    rendered = render_liquid_simple(page_html, context)

    (SITE_DIR / "index.html").write_text(rendered, encoding="utf-8")
    (SITE_DIR / ".nojekyll").touch()
    print("  -> Primary Jekyll site built into _site/")


def merge_docusaurus_docs(baseurl: str = ""):
    """Copies Docusaurus build output into _site/docs/"""
    print("[2/3] Merging Docusaurus docs into _site/docs/...")
    docs_dest = SITE_DIR / "docs"
    if docs_dest.exists():
        shutil.rmtree(docs_dest)

    if not WEBSITE_BUILD_DIR.exists():
        print(f"ERROR: Docusaurus build directory {WEBSITE_BUILD_DIR} does not exist. Run 'npm --prefix website run build' first.")
        sys.exit(1)

    shutil.copytree(WEBSITE_BUILD_DIR, docs_dest)
    print("  -> Docusaurus docs merged into _site/docs/")


def verify_site_structure():
    """Validates the output directory has all required assets and pages."""
    print("[3/3] Verifying assembled site...")
    assert (SITE_DIR / "index.html").exists(), "Missing _site/index.html"
    assert (SITE_DIR / ".nojekyll").exists(), "Missing _site/.nojekyll"
    assert (SITE_DIR / "assets" / "css" / "style.css").exists(), "Missing _site/assets/css/style.css"
    assert (SITE_DIR / "docs" / "index.html").exists(), "Missing _site/docs/index.html"
    assert (SITE_DIR / "docs" / "architecture" / "overview.html").exists() or (SITE_DIR / "docs" / "architecture" / "overview" / "index.html").exists(), "Missing docs architecture overview page"
    print("  -> Verification complete! All files and routes intact.")


if __name__ == "__main__":
    base = sys.argv[1] if len(sys.argv) > 1 else ""
    build_jekyll_offline(base)
    merge_docusaurus_docs(base)
    verify_site_structure()
