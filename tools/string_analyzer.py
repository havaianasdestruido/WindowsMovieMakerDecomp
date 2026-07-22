#!/usr/bin/env python3
"""
String Analyzer - Extract and compare strings from PE binaries.
Useful for finding UI text, error messages, and hidden strings.

Usage:
    python tools/string_analyzer.py <binary>
    python tools/string_analyzer.py --compare <original> <recreation>
    python tools/string_analyzer.py --wide <binary>
"""

import struct
import sys
import os
import re
import json
from pathlib import Path
from collections import Counter
from typing import List, Set, Dict, Tuple


def extract_strings(filepath: str, min_len: int = 4) -> Dict[str, List[str]]:
    """Extract both ASCII and wide strings from binary."""
    with open(filepath, "rb") as f:
        data = f.read()

    ascii_strings = []
    wide_strings = []

    # ASCII strings
    pattern = rb"[\x20-\x7e]{" + str(min_len).encode() + rb",}"
    for match in re.finditer(pattern, data):
        ascii_strings.append(match.group().decode("ascii"))

    # Wide strings (UTF-16LE)
    pattern = rb"(?:[\x20-\x7e]\x00){" + str(min_len).encode() + rb",}"
    for match in re.finditer(pattern, data):
        wide = match.group().decode("utf-16-le", errors="replace")
        wide_strings.append(wide)

    return {
        "ascii": sorted(set(ascii_strings)),
        "wide": sorted(set(wide_strings)),
    }


def categorize_strings(strings: List[str]) -> Dict[str, List[str]]:
    """Categorize strings by type."""
    categories = {
        "urls": [],
        "file_paths": [],
        "error_messages": [],
        "dll_names": [],
        "com_interfaces": [],
        "registry_keys": [],
        "format_strings": [],
        "class_names": [],
        "ui_text": [],
        "technical": [],
        "other": [],
    }

    url_pattern = re.compile(r"https?://|www\.|\.com|\.dll|\.exe|\.dll|\.xml|\.wlmp")
    path_pattern = re.compile(r"[A-Z]:\\|\\\\|\.dll|\.exe|\.sys|\.txt|\.xml")
    error_pattern = re.compile(r"error|fail|invalid|cannot|unable|exception", re.IGNORECASE)
    dll_pattern = re.compile(r"\.dll$", re.IGNORECASE)
    com_pattern = re.compile(r"I[A-Z][a-z]+|CLSID|IID_|__uuidof")
    registry_pattern = re.compile(r"HKEY_|HKLM|HKCU|Software\\\\|Software\\")
    format_pattern = re.compile(r"%[sdx]|%\.\d+f|\{0\}|\{1\}")
    class_pattern = re.compile(r"^[A-Z][a-zA-Z]+$|C[A-Z][a-z]+|CCom|Dui")
    ui_pattern = re.compile(r"button|menu|file|edit|view|help|save|open|close|import|export|play|pause|stop", re.IGNORECASE)

    for s in strings:
        if url_pattern.search(s):
            categories["urls"].append(s)
        elif path_pattern.search(s):
            categories["file_paths"].append(s)
        elif error_pattern.search(s):
            categories["error_messages"].append(s)
        elif dll_pattern.search(s):
            categories["dll_names"].append(s)
        elif com_pattern.search(s):
            categories["com_interfaces"].append(s)
        elif registry_pattern.search(s):
            categories["registry_keys"].append(s)
        elif format_pattern.search(s):
            categories["format_strings"].append(s)
        elif class_pattern.match(s):
            categories["class_names"].append(s)
        elif ui_pattern.search(s):
            categories["ui_text"].append(s)
        elif s.startswith("?AV") or s.startswith("?AU") or "??" in s:
            categories["technical"].append(s)
        else:
            categories["other"].append(s)

    return categories


def print_string_report(filepath: str, strings: Dict[str, List[str]]):
    print(f"\n{'='*70}")
    print(f"  String Analysis: {os.path.basename(filepath)}")
    print(f"{'='*70}")

    print(f"\n  ASCII strings: {len(strings['ascii'])}")
    print(f"  Wide strings:  {len(strings['wide'])}")

    all_strings = strings["ascii"] + strings["wide"]
    categories = categorize_strings(all_strings)

    print(f"\n  --- By Category ---")
    for cat, items in sorted(categories.items(), key=lambda x: -len(x[1])):
        if items:
            print(f"  {cat:<20} {len(items):>5}")

    # Show interesting strings
    if categories["urls"]:
        print(f"\n  --- URLs ({len(categories['urls'])}) ---")
        for s in categories["urls"][:20]:
            print(f"    {s}")

    if categories["dll_names"]:
        print(f"\n  --- DLL Names ({len(categories['dll_names'])}) ---")
        for s in sorted(set(categories["dll_names"])):
            print(f"    {s}")

    if categories["error_messages"]:
        print(f"\n  --- Error Messages ({len(categories['error_messages'])}) ---")
        for s in categories["error_messages"][:20]:
            print(f"    {s}")

    if categories["file_paths"]:
        print(f"\n  --- File Paths ({len(categories['file_paths'])}) ---")
        for s in categories["file_paths"][:20]:
            print(f"    {s}")

    if categories["registry_keys"]:
        print(f"\n  --- Registry Keys ({len(categories['registry_keys'])}) ---")
        for s in categories["registry_keys"][:10]:
            print(f"    {s}")

    print()


def compare_strings(orig: Dict[str, List[str]], recon: Dict[str, List[str]], orig_name: str, recon_name: str):
    print(f"\n{'='*70}")
    print(f"  String Comparison: {orig_name} vs {recon_name}")
    print(f"{'='*70}")

    orig_set = set(orig["ascii"] + orig["wide"])
    recon_set = set(recon["ascii"] + recon["wide"])

    missing = orig_set - recon_set
    extra = recon_set - orig_set
    common = orig_set & recon_set

    print(f"\n  Original strings:  {len(orig_set)}")
    print(f"  Recreation strings: {len(recon_set)}")
    print(f"  Common:            {len(common)}")
    print(f"  Missing:           {len(missing)}")
    print(f"  Extra:             {len(extra)}")
    print(f"  Coverage:          {len(common)/len(orig_set)*100:.1f}%" if orig_set else "")

    if missing:
        print(f"\n  --- Missing Strings (first 30) ---")
        for s in sorted(missing)[:30]:
            print(f"    - {s[:100]}")

    print()


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    if sys.argv[1] == "--compare" and len(sys.argv) >= 4:
        orig = extract_strings(sys.argv[2])
        recon = extract_strings(sys.argv[3])
        print_string_report(sys.argv[2], orig)
        print_string_report(sys.argv[3], recon)
        compare_strings(orig, recon, os.path.basename(sys.argv[2]), os.path.basename(sys.argv[3]))
    else:
        strings = extract_strings(sys.argv[1])
        print_string_report(sys.argv[1], strings)

        report_path = os.path.join("tools", "reports", f"{os.path.basename(sys.argv[1])}_strings.json")
        os.makedirs(os.path.dirname(report_path), exist_ok=True)
        with open(report_path, "w") as f:
            json.dump(strings, f, indent=2)
        print(f"  Report saved to {report_path}")
