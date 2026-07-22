#!/usr/bin/env python3
"""
RTTI Extractor - Extract and analyze MSVC RTTI class names from PE binaries.
Uses RTTICompleteObjectLocator structures to find all polymorphic classes.

Usage:
    python tools/rtti_extractor.py <binary>
    python tools/rtti_extractor.py --compare <original> <recreation>
    python tools/rtti_extractor.py --match <binary> <source_dir>
"""

import struct
import sys
import os
import re
import json
from pathlib import Path
from collections import Counter
from dataclasses import dataclass, field, asdict
from typing import List, Dict, Set, Optional, Tuple


@dataclass
class RTTIInfo:
    class_name: str
    mangled: str
    rva: int
    offset: int = 0
    vtable_rva: int = 0
    base_classes: List[str] = field(default_factory=list)


def undecorate_msvc_name(mangled: str) -> str:
    """Best-effort MSVC name demangling."""
    name = mangled

    # Remove ?AV prefix (class)
    if name.startswith("?AV"):
        name = name[3:]
    elif name.startswith("?A"):  # union
        name = name[2:]

    # Remove @@ suffix
    if name.endswith("@@"):
        name = name[:-2]

    # Replace @ with ::
    name = name.replace("@", "::")

    # Remove leading ? prefix
    if name.startswith("?"):
        name = name[1:]

    return name


def extract_rtti_strings(binary_path: str) -> List[RTTIInfo]:
    """Extract RTTI class names by scanning for TypeDescriptor patterns."""
    with open(binary_path, "rb") as f:
        data = f.read()

    results = []
    seen = set()

    # MSVC RTTI TypeDescriptor pattern:
    # The type_info vtable pointer followed by the mangled name
    # We scan for "?AV" patterns which indicate class type names
    # The mangled names are null-terminated strings in .rdata

    # Pattern 1: Direct string scan for class names
    pattern = rb"\?AV[^\x00]{3,200}\x00"
    for match in re.finditer(pattern, data):
        mangled = match.group().rstrip(b"\x00").decode("ascii", errors="replace")
        rva = match.start()

        if mangled not in seen:
            seen.add(mangled)
            class_name = undecorate_msvc_name(mangled)
            results.append(RTTIInfo(
                class_name=class_name,
                mangled=mangled,
                rva=rva,
            ))

    # Pattern 2: Look for union types
    pattern2 = rb"\?AU[^\x00]{3,200}\x00"
    for match in re.finditer(pattern2, data):
        mangled = match.group().rstrip(b"\x00").decode("ascii", errors="replace")
        rva = match.start()

        if mangled not in seen:
            seen.add(mangled)
            class_name = undecorate_msvc_name(mangled)
            results.append(RTTIInfo(
                class_name=class_name,
                mangled=mangled,
                rva=rva,
            ))

    # Pattern 3: Nested/local classes (?A?1?? pattern)
    pattern3 = rb"\?A\?[^\x00]{3,300}\x00"
    for match in re.finditer(pattern3, data):
        mangled = match.group().rstrip(b"\x00").decode("ascii", errors="replace")
        rva = match.start()

        if mangled not in seen:
            seen.add(mangled)
            class_name = undecorate_msvc_name(mangled)
            results.append(RTTIInfo(
                class_name=class_name,
                mangled=mangled,
                rva=rva,
            ))

    return sorted(results, key=lambda r: r.rva)


def extract_rtti_structures(binary_path: str) -> List[RTTIInfo]:
    """Extract RTTI by scanning for CompleteObjectLocator patterns."""
    with open(binary_path, "rb") as f:
        data = f.read()

    results = []

    # Look for common vtable signatures (pointers to type_info)
    # RTTI::CompleteObjectLocator has:
    #   signature (4 bytes, usually 0 or 1)
    #   offset (4 bytes)
    #   cdOffset (4 bytes)
    #   TypeDescriptor RVA (4 bytes)

    # This is harder to do generically, so we rely mainly on string extraction
    # but try to find vtable pointers too

    # Scan for ??_R0 (type_info object prefix in MSVC)
    pattern = rb"\?\?_R0[^\x00]{2,100}\x00"
    seen = set()
    for match in re.finditer(pattern, data):
        mangled = match.group().rstrip(b"\x00").decode("ascii", errors="replace")
        rva = match.start()
        if mangled not in seen:
            seen.add(mangled)
            # ??_R0?AVFoo@@ = type_info for class Foo
            clean = mangled.replace("??_R0", "")
            class_name = undecorate_msvc_name(clean)
            results.append(RTTIInfo(
                class_name=class_name,
                mangled=mangled,
                rva=rva,
            ))

    return results


def scan_vtables(binary_path: str) -> Dict[int, List[int]]:
    """Scan for vtable patterns (arrays of function pointers)."""
    with open(binary_path, "rb") as f:
        data = f.read()

    vtables = {}

    # Look for patterns: multiple consecutive RVAs that look like code pointers
    # This is a heuristic - real vtable detection needs PE section mapping
    for i in range(0, len(data) - 16, 4):
        val1 = struct.unpack_from("<I", data, i)[0]
        if val1 < 0x1000 or val1 > 0x100000:
            continue

        # Check if next few values also look like function pointers
        consecutive = 0
        for j in range(1, 20):
            if i + j * 4 + 4 > len(data):
                break
            next_val = struct.unpack_from("<I", data, i + j * 4)[0]
            if 0x1000 <= next_val <= 0x100000:
                consecutive += 1
            else:
                break

        if consecutive >= 3:
            vtables[i] = [val1] + [
                struct.unpack_from("<I", data, i + j * 4)[0]
                for j in range(1, consecutive + 1)
            ]

    return vtables


def print_rtti_report(results: List[RTTIInfo], binary_name: str):
    print(f"\n{'='*70}")
    print(f"  RTTI Analysis: {binary_name}")
    print(f"  Found {len(results)} unique class/type names")
    print(f"{'='*70}\n")

    # Group by category
    classes = [r for r in results if "?AV" in r.mangled or r.mangled.startswith("?AV")]
    unions = [r for r in results if "?AU" in r.mangled]
    nested = [r for r in results if "?A?" in r.mangled]
    r0_types = [r for r in results if "??_R0" in r.mangled]
    other = [r for r in results if r not in classes and r not in unions and r not in nested and r not in r0_types]

    print(f"  Classes:  {len(classes)}")
    print(f"  Unions:   {len(unions)}")
    print(f"  Nested:   {len(nested)}")
    print(f"  type_info: {len(r0_types)}")
    print(f"  Other:    {len(other)}")

    print(f"\n  --- Class Names (first 50) ---")
    for r in sorted(classes, key=lambda x: x.class_name)[:50]:
        print(f"    {r.class_name}")

    if len(classes) > 50:
        print(f"    ... and {len(classes) - 50} more")

    if unions:
        print(f"\n  --- Union Names ---")
        for r in sorted(unions, key=lambda x: x.class_name):
            print(f"    {r.class_name}")

    if nested:
        print(f"\n  --- Nested/Local Class Names (first 30) ---")
        for r in sorted(nested, key=lambda x: x.class_name)[:30]:
            print(f"    {r.class_name}")

    print()


def compare_rtti(orig_names: Set[str], recon_names: Set[str]):
    print(f"\n{'='*70}")
    print(f"  RTTI Comparison")
    print(f"{'='*70}")

    missing = orig_names - recon_names
    extra = recon_names - orig_names
    common = orig_names & recon_names

    coverage = (len(common) / len(orig_names) * 100) if orig_names else 0

    print(f"\n  Original classes:  {len(orig_names)}")
    print(f"  Recreation classes: {len(recon_names)}")
    print(f"  Common:            {len(common)}")
    print(f"  Coverage:          {coverage:.1f}%")
    print(f"  Missing:           {len(missing)}")
    print(f"  Extra:             {len(extra)}")

    if missing:
        print(f"\n  --- Missing from Recreation ---")
        for name in sorted(missing)[:30]:
            print(f"    - {name}")
        if len(missing) > 30:
            print(f"    ... and {len(missing) - 30} more")

    if extra:
        print(f"\n  --- Extra in Recreation ---")
        for name in sorted(extra)[:30]:
            print(f"    + {name}")
        if len(extra) > 30:
            print(f"    ... and {len(extra) - 30} more")

    print()


def match_source_files(rtti_results: List[RTTIInfo], source_dir: str):
    """Match RTTI names to source files containing them."""
    print(f"\n{'='*70}")
    print(f"  RTTI -> Source File Matching")
    print(f"{'='*70}\n")

    # Build source index
    source_classes = {}
    for cpp_file in Path(source_dir).rglob("*.h"):
        try:
            content = cpp_file.read_text(errors="replace")
            for match in re.finditer(r"class\s+(\w+)", content):
                source_classes[match.group(1)] = str(cpp_file)
        except:
            pass

    for cpp_file in Path(source_dir).rglob("*.cpp"):
        try:
            content = cpp_file.read_text(errors="replace")
            for match in re.finditer(r"class\s+(\w+)", content):
                if match.group(1) not in source_classes:
                    source_classes[match.group(1)] = str(cpp_file)
        except:
            pass

    matched = 0
    unmatched = 0
    for rtti in rtti_results:
        class_name = rtti.class_name.split("::")[-1] if "::" in rtti.class_name else rtti.class_name
        if class_name in source_classes:
            print(f"    {rtti.class_name:<50} -> {source_classes[class_name]}")
            matched += 1
        else:
            unmatched += 1

    print(f"\n  Matched: {matched} | Unmatched: {unmatched}")
    print()


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    if sys.argv[1] == "--compare" and len(sys.argv) >= 4:
        orig = extract_rtti_strings(sys.argv[2])
        recon = extract_rtti_strings(sys.argv[3])
        print_rtti_report(orig, os.path.basename(sys.argv[2]))
        print_rtti_report(recon, os.path.basename(sys.argv[3]))
        compare_rtti(
            {r.class_name for r in orig},
            {r.class_name for r in recon},
        )
    elif sys.argv[1] == "--match" and len(sys.argv) >= 4:
        results = extract_rtti_strings(sys.argv[2])
        match_source_files(results, sys.argv[3])
    else:
        results = extract_rtti_strings(sys.argv[1])
        print_rtti_report(results, os.path.basename(sys.argv[1]))

        # Also try vtable scanning
        vtables = scan_vtables(sys.argv[1])
        print(f"  Potential vtables found: {len(vtables)}")

        # Save report
        report_path = os.path.join("tools", "reports", f"{os.path.basename(sys.argv[1])}_rtti.json")
        os.makedirs(os.path.dirname(report_path), exist_ok=True)
        with open(report_path, "w") as f:
            json.dump([asdict(r) for r in results], f, indent=2)
        print(f"\n  Report saved to {report_path}")
