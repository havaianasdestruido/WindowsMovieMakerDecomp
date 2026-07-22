#!/usr/bin/env python3
"""
Export/Import Diff Tool - Compare export and import tables between
original and recreation binaries.

Usage:
    python tools/diff_exports.py <original.exe> <recreation.dll>
    python tools/diff_exports.py --batch <orig_dir> <recon_dir>
"""

import struct
import sys
import os
import json
from pathlib import Path
from dataclasses import dataclass, field
from typing import List, Set, Dict, Tuple


def parse_exports(filepath: str) -> Dict[str, int]:
    """Parse PE export table, return {name: ordinal}."""
    exports = {}
    try:
        with open(filepath, "rb") as f:
            dos_sig = struct.unpack("<H", f.read(2))[0]
            if dos_sig != 0x5A4D:
                return exports

            f.seek(0x3C)
            pe_offset = struct.unpack("<I", f.read(4))[0]
            f.seek(pe_offset + 4)
            machine = struct.unpack("<H", f.read(2))[0]
            is_64 = (machine == 0x8664)

            f.seek(pe_offset + 24)
            opt_magic = struct.unpack("<H", f.read(2))[0]
            if opt_magic == 0x20B:
                data_dir_offset = pe_offset + 24 + 112
            else:
                data_dir_offset = pe_offset + 24 + 96

            f.seek(data_dir_offset)
            export_rva = struct.unpack("<I", f.read(4))[0]
            export_size = struct.unpack("<I", f.read(4))[0]

            if export_rva == 0:
                return exports

            # Find section
            f.seek(pe_offset + 6)
            num_sections = struct.unpack("<H", f.read(2))[0]
            f.seek(pe_offset + 20)
            opt_size = struct.unpack("<H", f.read(2))[0]
            sections_start = pe_offset + 24 + opt_size

            sections = []
            f.seek(sections_start)
            for _ in range(num_sections):
                name = f.read(8).rstrip(b"\x00").decode("ascii", errors="replace")
                vs = struct.unpack("<I", f.read(4))[0]
                va = struct.unpack("<I", f.read(4))[0]
                rs = struct.unpack("<I", f.read(4))[0]
                ro = struct.unpack("<I", f.read(4))[0]
                f.read(12)
                ch = struct.unpack("<I", f.read(4))[0]
                sections.append((name, va, vs, ro, rs, ch))

            def rva_to_off(rva):
                for _, va, vs, ro, rs, _ in sections:
                    if va <= rva < va + vs:
                        return rva - va + ro
                return None

            off = rva_to_off(export_rva)
            if not off:
                return exports

            f.seek(off + 20)
            num_names = struct.unpack("<I", f.read(4))[0]
            funcs_rva = struct.unpack("<I", f.read(4))[0]
            names_rva = struct.unpack("<I", f.read(4))[0]
            ords_rva = struct.unpack("<I", f.read(4))[0]
            base = struct.unpack("<I", f.read(4))[0]

            # Read function RVAs
            func_off = rva_to_off(funcs_rva)
            if not func_off:
                return exports
            f.seek(func_off)
            func_rvas = [struct.unpack("<I", f.read(4))[0] for _ in range(struct.unpack("<I", f.read(4))[0] if False else num_names + 100)]

            # Actually read num_functions from export dir
            f.seek(off + 20)
            _ = struct.unpack("<I", f.read(4))[0]  # num_names again
            num_funcs = struct.unpack("<I", f.read(4))[0]
            f.seek(func_off)
            func_rvas = []
            for _ in range(num_funcs):
                func_rvas.append(struct.unpack("<I", f.read(4))[0])

            name_off = rva_to_off(names_rva)
            ord_off = rva_to_off(ords_rva)

            if name_off and ord_off:
                for i in range(num_names):
                    f.seek(name_off + i * 4)
                    n_rva = struct.unpack("<I", f.read(4))[0]
                    f.seek(ord_off + i * 2)
                    idx = struct.unpack("<H", f.read(2))[0]

                    n_off = rva_to_off(n_rva)
                    if n_off:
                        f.seek(n_off)
                        name = b""
                        while True:
                            b = f.read(1)
                            if not b or b == b"\x00":
                                break
                            name += b
                        exports[name.decode("ascii", errors="replace")] = base + idx

    except Exception as e:
        print(f"  Error parsing {filepath}: {e}")

    return exports


def parse_imports(filepath: str) -> Set[str]:
    """Parse PE import table, return set of DLL names."""
    imports = set()
    try:
        with open(filepath, "rb") as f:
            dos_sig = struct.unpack("<H", f.read(2))[0]
            if dos_sig != 0x5A4D:
                return imports

            f.seek(0x3C)
            pe_offset = struct.unpack("<I", f.read(4))[0]
            f.seek(pe_offset + 4)
            machine = struct.unpack("<H", f.read(2))[0]
            is_64 = (machine == 0x8664)

            f.seek(pe_offset + 24)
            opt_magic = struct.unpack("<H", f.read(2))[0]
            if opt_magic == 0x20B:
                data_dir_offset = pe_offset + 24 + 112
            else:
                data_dir_offset = pe_offset + 24 + 96

            f.seek(data_dir_offset + 8)  # skip export
            import_rva = struct.unpack("<I", f.read(4))[0]
            import_size = struct.unpack("<I", f.read(4))[0]

            if import_rva == 0:
                return imports

            f.seek(pe_offset + 6)
            num_sections = struct.unpack("<H", f.read(2))[0]
            f.seek(pe_offset + 20)
            opt_size = struct.unpack("<H", f.read(2))[0]

            sections = []
            f.seek(pe_offset + 24 + opt_size)
            for _ in range(num_sections):
                name = f.read(8).rstrip(b"\x00").decode("ascii", errors="replace")
                vs = struct.unpack("<I", f.read(4))[0]
                va = struct.unpack("<I", f.read(4))[0]
                rs = struct.unpack("<I", f.read(4))[0]
                ro = struct.unpack("<I", f.read(4))[0]
                f.read(12)
                ch = struct.unpack("<I", f.read(4))[0]
                sections.append((name, va, vs, ro, rs, ch))

            def rva_to_off(rva):
                for _, va, vs, ro, rs, _ in sections:
                    if va <= rva < va + vs:
                        return rva - va + ro
                return None

            off = rva_to_off(import_rva)
            if not off:
                return imports

            f.seek(off)
            while True:
                _ = struct.unpack("<I", f.read(4))[0]
                _ = struct.unpack("<I", f.read(4))[0]
                _ = struct.unpack("<H", f.read(2))[0]
                _ = struct.unpack("<H", f.read(2))[0]
                _ = struct.unpack("<H", f.read(2))[0]
                dll_rva = struct.unpack("<I", f.read(4))[0]
                _ = struct.unpack("<I", f.read(4))[0]

                if dll_rva == 0:
                    break

                dll_off = rva_to_off(dll_rva)
                if not dll_off:
                    break

                f.seek(dll_off)
                dll_name = b""
                while True:
                    b = f.read(1)
                    if not b or b == b"\x00":
                        break
                    dll_name += b
                imports.add(dll_name.decode("ascii", errors="replace").lower())

    except Exception as e:
        print(f"  Error parsing {filepath}: {e}")

    return imports


def diff_binary(orig_path: str, recon_path: str):
    print(f"\n{'='*70}")
    print(f"  Export/Import Diff")
    print(f"  Original:  {os.path.basename(orig_path)}")
    print(f"  Recreation: {os.path.basename(recon_path)}")
    print(f"{'='*70}")

    orig_exports = parse_exports(orig_path)
    recon_exports = parse_exports(recon_path)

    print(f"\n  --- Exports ---")
    print(f"  Original:  {len(orig_exports)} exports")
    print(f"  Recreation: {len(recon_exports)} exports")

    orig_names = set(orig_exports.keys())
    recon_names = set(recon_exports.keys())
    missing = orig_names - recon_names
    extra = recon_names - orig_names
    common = orig_names & recon_names

    print(f"  Common:    {len(common)}")
    print(f"  Missing:   {len(missing)}")
    print(f"  Extra:     {len(extra)}")
    print(f"  Coverage:  {len(common)/len(orig_names)*100:.1f}%" if orig_names else "  Coverage: N/A")

    if missing:
        print(f"\n  Missing exports:")
        for name in sorted(missing):
            print(f"    - {name} (ordinal {orig_exports.get(name, '?')})")

    if extra:
        print(f"\n  Extra exports in recreation:")
        for name in sorted(extra):
            print(f"    + {name}")

    # Import diff
    orig_imports = parse_imports(orig_path)
    recon_imports = parse_imports(recon_path)

    print(f"\n  --- Imports ---")
    print(f"  Original DLLs:  {sorted(orig_imports)}")
    print(f"  Recreation DLLs: {sorted(recon_imports)}")

    missing_dlls = orig_imports - recon_imports
    extra_dlls = recon_imports - orig_imports
    if missing_dlls:
        print(f"  Missing DLLs: {sorted(missing_dlls)}")
    if extra_dlls:
        print(f"  Extra DLLs: {sorted(extra_dlls)}")

    print()


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)

    if sys.argv[1] == "--batch" and len(sys.argv) >= 4:
        orig_dir = sys.argv[2]
        recon_dir = sys.argv[3]

        orig_files = {}
        for f in Path(orig_dir).glob("*.dll"):
            orig_files[f.stem.lower()] = str(f)
        for f in Path(orig_dir).glob("*.exe"):
            orig_files[f.stem.lower()] = str(f)

        recon_files = {}
        for f in Path(recon_dir).glob("*.dll"):
            recon_files[f.stem.lower()] = str(f)
        for f in Path(recon_dir).glob("*.exe"):
            recon_files[f.stem.lower()] = str(f)

        for name in sorted(set(orig_files.keys()) & set(recon_files.keys())):
            diff_binary(orig_files[name], recon_files[name])
    else:
        diff_binary(sys.argv[1], sys.argv[2])
