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
from pathlib import Path
from typing import Set, Dict


class PEFormatError(ValueError):
    """Malformed/truncated PE input; never treat this as an empty table."""


class PEReader:
    def __init__(self, filepath):
        self.data = Path(filepath).read_bytes()
        if self.read(0, 2) != b"MZ":
            raise PEFormatError("Missing DOS signature")
        pe = self.unpack("<I", 0x3c)[0]
        if self.read(pe, 4) != b"PE\0\0":
            raise PEFormatError("Missing PE signature")
        count = self.unpack("<H", pe + 6)[0]
        opt_size = self.unpack("<H", pe + 20)[0]
        opt = pe + 24
        self.read(opt, opt_size)
        magic = self.unpack("<H", opt)[0]
        if magic not in (0x10b, 0x20b):
            raise PEFormatError("Unsupported optional header")
        directory_offset = 96 if magic == 0x10b else 112
        if opt_size < directory_offset:
            raise PEFormatError("Short optional header")
        self.header_size = self.unpack("<I", opt + 60)[0]
        directory_count = self.unpack("<I", opt + directory_offset - 4)[0]
        if directory_count > (opt_size - directory_offset) // 8:
            raise PEFormatError("Truncated data directories")
        self.directories = [self.unpack("<II", opt + directory_offset + n * 8)
                            for n in range(directory_count)]
        self.sections = []
        for n in range(count):
            off = opt + opt_size + n * 40
            self.read(off, 40)
            vs, va, raw_size, raw_off = self.unpack("<IIII", off + 8)
            self.read(raw_off, raw_size)
            self.sections.append((va, raw_off, raw_size))

    def read(self, off, size):
        if off < 0 or size < 0 or off > len(self.data) or size > len(self.data) - off:
            raise PEFormatError("Read outside file")
        return self.data[off:off + size]

    def unpack(self, fmt, off):
        return struct.unpack(fmt, self.read(off, struct.calcsize(fmt)))

    def region(self, rva):
        if 0 <= rva < min(self.header_size, len(self.data)):
            return rva, min(self.header_size, len(self.data)) - rva
        for va, raw_off, raw_size in self.sections:
            if va <= rva < va + raw_size:
                return raw_off + rva - va, raw_size - (rva - va)
        raise PEFormatError(f"Unmapped RVA 0x{rva:x}")

    def offset(self, rva, size):
        off, available = self.region(rva)
        if size > available:
            raise PEFormatError("RVA range exceeds raw section")
        self.read(off, size)
        return off

    def string(self, rva):
        off, available = self.region(rva)
        end = self.data.find(b"\0", off, off + available)
        if end < 0:
            raise PEFormatError("Unterminated PE string")
        return self.data[off:end].decode("ascii", errors="strict")

    def directory(self, index):
        rva, size = self.directories[index] if index < len(self.directories) else (0, 0)
        if not rva and not size:
            return None
        if not rva or not size:
            raise PEFormatError("Incomplete data directory")
        return self.offset(rva, size), size


def parse_exports(filepath: str) -> Dict[str, int]:
    """Named exports and ordinal-only exports (#N), preserving aliases/holes (#26)."""
    pe = PEReader(filepath)
    directory = pe.directory(0)
    if directory is None:
        return {}
    off, size = directory
    if size < 40:
        raise PEFormatError("Short export directory")
    base, num_funcs, num_names, funcs, names, ords = pe.unpack("<IIIIII", off + 16)
    func_off = pe.offset(funcs, num_funcs * 4) if num_funcs else 0
    name_off = pe.offset(names, num_names * 4) if num_names else 0
    ord_off = pe.offset(ords, num_names * 2) if num_names else 0
    exports, named = {}, set()
    for i in range(num_names):
        idx = pe.unpack("<H", ord_off + i * 2)[0]
        if idx >= num_funcs or not pe.unpack("<I", func_off + idx * 4)[0]:
            raise PEFormatError("Invalid export ordinal index")
        name = pe.string(pe.unpack("<I", name_off + i * 4)[0])
        if name in exports:
            raise PEFormatError("Duplicate export name")
        exports[name] = base + idx
        named.add(idx)
    for idx in range(num_funcs):
        if idx not in named and pe.unpack("<I", func_off + idx * 4)[0]:
            exports[f"#{base + idx}"] = base + idx
    return exports


def parse_imports(filepath: str) -> Set[str]:
    pe = PEReader(filepath)
    directory = pe.directory(1)
    if directory is None:
        return set()
    off, size = directory
    imports = set()
    for cursor in range(0, size - 19, 20):
        descriptor = pe.unpack("<IIIII", off + cursor)
        if not any(descriptor):
            return imports
        dll_rva = descriptor[3]
        if not dll_rva:
            raise PEFormatError("Import descriptor without DLL name")
        imports.add(pe.string(dll_rva).lower())
    raise PEFormatError("Unterminated import descriptor table")


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
    changed_ordinals = {name for name in common if orig_exports[name] != recon_exports[name]}

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

    if changed_ordinals:
        print(f"  Changed export ordinals: {sorted(changed_ordinals)}")

    # Import differences are informational: reconstructed implementation dependencies may differ.
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
    return not (missing or extra or changed_ordinals)


def main(argv=None):
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--batch", action="store_true")
    parser.add_argument("--exit-zero", action="store_true", help="Report-only: ignore parity differences, not input errors")
    parser.add_argument("original")
    parser.add_argument("recreation")
    args = parser.parse_args(argv)
    try:
        pairs = [(args.original, args.recreation)]
        missing_files = False
        if args.batch:
            def binaries(directory):
                path = Path(directory)
                if not path.is_dir():
                    raise PEFormatError(f"Not a directory: {path}")
                return {p.name.lower(): p for p in path.iterdir()
                        if p.suffix.lower() in (".dll", ".exe") and p.is_file()}
            original, recreation = binaries(args.original), binaries(args.recreation)
            if not original or not recreation:
                raise PEFormatError("No binaries to compare")
            missing = set(original) ^ set(recreation)
            if missing:
                print(f"Unpaired binaries: {sorted(missing)}", file=sys.stderr)
                missing_files = True
            pairs = [(original[n], recreation[n]) for n in sorted(set(original) & set(recreation))]
        matched = not missing_files
        for original, recreation in pairs:
            matched = diff_binary(str(original), str(recreation)) and matched
        if missing_files:
            return 1
        return 0 if matched or args.exit_zero else 1
    except (OSError, ValueError, struct.error) as error:
        print(f"PE comparison failed: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
