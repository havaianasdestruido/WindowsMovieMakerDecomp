#!/usr/bin/env python3
"""
PE Analyzer - Advanced Portable Executable analysis tool
For studying Windows Live Movie Maker 2012 binaries and our recreation.

Usage:
    python tools/pe_analyzer.py <path_to_exe_or_dll>
    python tools/pe_analyzer.py --compare <original> <recreation>
    python tools/pe_analyzer.py --batch <directory>
"""

import struct
import sys
import os
import json
from pathlib import Path
from dataclasses import dataclass, field, asdict
from typing import List, Optional, Dict, Tuple
from datetime import datetime, timedelta

# PE constants
IMAGE_DOS_SIGNATURE = 0x5A4D
IMAGE_NT_SIGNATURE = 0x00004550
IMAGE_FILE_MACHINE_I386 = 0x014C
IMAGE_FILE_MACHINE_AMD64 = 0x8664
IMAGE_DIRECTORY_ENTRY_EXPORT = 0
IMAGE_DIRECTORY_ENTRY_IMPORT = 1
IMAGE_DIRECTORY_ENTRY_RESOURCE = 2
IMAGE_DIRECTORY_ENTRY_DEBUG = 6
IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT = 13
IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR = 14

SECTION_NAMES = {
    '.text': 'Code',
    '.rdata': 'Read-only Data',
    '.data': 'Data',
    '.rsrc': 'Resources',
    '.reloc': 'Relocations',
    '.idata': 'Import Data',
    '.edata': 'Export Data',
    '.pdata': 'Exception Data',
    '.tls': 'Thread Local Storage',
    '.CRT': 'C Runtime',
    '.debug': 'Debug Info',
}


@dataclass
class PEExport:
    name: str
    ordinal: int
    rva: int
    forwarded: bool = False
    forward_name: str = ""


@dataclass
class PEImport:
    dll_name: str
    functions: List[str] = field(default_factory=list)
    ordinal_imports: List[int] = field(default_factory=list)
    is_delay: bool = False


@dataclass
class PESection:
    name: str
    virtual_address: int
    virtual_size: int
    raw_offset: int
    raw_size: int
    characteristics: int
    entropy: float = 0.0

    @property
    def is_code(self): return bool(self.characteristics & 0x20)
    @property
    def is_executable(self): return bool(self.characteristics & 0x20000000)
    @property
    def is_readable(self): return bool(self.characteristics & 0x40000000)
    @property
    def is_writable(self): return bool(self.characteristics & 0x80000000)
    @property
    def is_initialized(self): return bool(self.characteristics & 0x00000040)


@dataclass
class PEData:
    filepath: str
    filename: str
    file_size: int
    is_64bit: bool
    machine: int
    subsystem: int
    timestamp: int
    image_base: int
    entry_point_rva: int
    sections: List[PESection] = field(default_factory=list)
    exports: List[PEExport] = field(default_factory=list)
    imports: List[PEImport] = field(default_factory=list)
    delay_imports: List[PEImport] = field(default_factory=list)
    resource_types: Dict[str, int] = field(default_factory=dict)
    debug_info: Dict[str, str] = field(default_factory=dict)
    rich_header: Dict[str, int] = field(default_factory=dict)
    anomalies: List[str] = field(default_factory=list)

    def to_dict(self):
        d = asdict(self)
        return d


def calc_entropy(data: bytes) -> float:
    if not data:
        return 0.0
    freq = [0] * 256
    for b in data:
        freq[b] += 1
    length = len(data)
    entropy = 0.0
    for count in freq:
        if count > 0:
            p = count / length
            import math
            entropy -= p * math.log2(p)
    return round(entropy, 4)


def read_cstring(f, offset=None):
    if offset is not None:
        f.seek(offset)
    result = b""
    while True:
        b = f.read(1)
        if not b or b == b"\x00":
            break
        result += b
    return result.decode("ascii", errors="replace")


def parse_pe(filepath: str) -> PEData:
    with open(filepath, "rb") as f:
        data = PEData(
            filepath=filepath,
            filename=os.path.basename(filepath),
            file_size=os.path.getsize(filepath),
            is_64bit=False,
            machine=0,
            subsystem=0,
            timestamp=0,
            image_base=0,
            entry_point_rva=0,
        )

        # DOS Header
        dos_sig = struct.unpack("<H", f.read(2))[0]
        if dos_sig != IMAGE_DOS_SIGNATURE:
            data.anomalies.append(f"Not a valid PE file (DOS sig: 0x{dos_sig:04X})")
            return data

        f.seek(0x3C)
        pe_offset = struct.unpack("<I", f.read(4))[0]

        # Rich Header detection
        f.seek(0x80)
        rich_data = f.read(min(pe_offset - 0x80, 0x200))
        if b"Rich" in rich_data:
            rich_pos = rich_data.find(b"Rich")
            # Look for DanS marker
            dans_pos = rich_data.find(b"DanS")
            if dans_pos >= 0:
                data.anomalies.append(f"Rich header detected at offset 0x{dans_pos + 0x80:X}")

        # PE Signature
        f.seek(pe_offset)
        pe_sig = struct.unpack("<I", f.read(4))[0]
        if pe_sig != IMAGE_NT_SIGNATURE:
            data.anomalies.append(f"Invalid PE signature: 0x{pe_sig:08X}")
            return data

        # COFF Header
        data.machine = struct.unpack("<H", f.read(2))[0]
        num_sections = struct.unpack("<H", f.read(2))[0]
        data.timestamp = struct.unpack("<I", f.read(4))[0]
        f.read(8)  # PointerToSymbolTable + NumberOfSymbols
        optional_header_size = struct.unpack("<H", f.read(2))[0]
        characteristics = struct.unpack("<H", f.read(2))[0]

        is_pe32plus = (data.machine == IMAGE_FILE_MACHINE_AMD64)
        data.is_64bit = is_pe32plus

        # Optional Header
        opt_start = f.tell()
        opt_magic = struct.unpack("<H", f.read(2))[0]

        if opt_magic == 0x20B:  # PE32+
            data.is_64bit = True
            f.read(2)  # MajorLinkerVersion, MinorLinkerVersion
            code_size = struct.unpack("<I", f.read(4))[0]
            f.read(4)  # SizeOfInitializedData
            f.read(4)  # SizeOfUninitializedData
            data.entry_point_rva = struct.unpack("<I", f.read(4))[0]
            f.read(4)  # BaseOfCode
            data.image_base = struct.unpack("<Q", f.read(8))[0]
        elif opt_magic == 0x10B:  # PE32
            data.is_64bit = False
            f.read(2)
            code_size = struct.unpack("<I", f.read(4))[0]
            f.read(4)
            f.read(4)
            data.entry_point_rva = struct.unpack("<I", f.read(4))[0]
            f.read(4)
            data.image_base = struct.unpack("<I", f.read(4))[0]
        else:
            data.anomalies.append(f"Unknown optional header magic: 0x{opt_magic:04X}")
            return data

        # Seek to section headers
        f.seek(opt_start + optional_header_size)

        # Section Headers
        for i in range(num_sections):
            raw_name = f.read(8)
            name = raw_name.rstrip(b"\x00").decode("ascii", errors="replace")
            virtual_size = struct.unpack("<I", f.read(4))[0]
            virtual_addr = struct.unpack("<I", f.read(4))[0]
            raw_size = struct.unpack("<I", f.read(4))[0]
            raw_offset = struct.unpack("<I", f.read(4))[0]
            f.read(12)  # Relocations, LineNumbers, etc.
            characteristics = struct.unpack("<I", f.read(4))[0]

            # Calculate entropy
            pos = f.tell()
            if raw_offset > 0 and raw_size > 0:
                f.seek(raw_offset)
                section_data = f.read(min(raw_size, 0x10000))
                entropy = calc_entropy(section_data)
            else:
                entropy = 0.0
            f.seek(pos)

            data.sections.append(PESection(
                name=name,
                virtual_address=virtual_addr,
                virtual_size=virtual_size,
                raw_offset=raw_offset,
                raw_size=raw_size,
                characteristics=characteristics,
                entropy=entropy,
            ))

        # Parse Exports
        f.seek(opt_start)
        if is_pe32plus:
            f.seek(opt_start + 112)  # DataDirectory offset for PE32+
        else:
            f.seek(opt_start + 96)  # DataDirectory offset for PE32

        data_dirs = []
        for _ in range(16):
            rva = struct.unpack("<I", f.read(4))[0]
            size = struct.unpack("<I", f.read(4))[0]
            data_dirs.append((rva, size))

        export_rva, export_size = data_dirs[IMAGE_DIRECTORY_ENTRY_EXPORT]
        if export_rva > 0:
            # Find section containing export RVA
            export_offset = rva_to_offset(data.sections, export_rva)
            if export_offset:
                f.seek(export_offset)
                _ = struct.unpack("<I", f.read(4))[0]  # Characteristics
                _ = struct.unpack("<I", f.read(4))[0]  # TimeDateStamp
                _ = struct.unpack("<H", f.read(2))[0]  # MajorVersion
                _ = struct.unpack("<H", f.read(2))[0]  # MinorVersion
                name_rva = struct.unpack("<I", f.read(4))[0]
                ordinal_base = struct.unpack("<I", f.read(4))[0]
                num_functions = struct.unpack("<I", f.read(4))[0]
                num_names = struct.unpack("<I", f.read(4))[0]
                functions_rva = struct.unpack("<I", f.read(4))[0]
                names_rva = struct.unpack("<I", f.read(4))[0]
                ordinals_rva = struct.unpack("<I", f.read(4))[0]

                # Read function RVAs
                func_rvas = []
                func_offset = rva_to_offset(data.sections, functions_rva)
                if func_offset:
                    f.seek(func_offset)
                    for _ in range(num_functions):
                        func_rvas.append(struct.unpack("<I", f.read(4))[0])

                # Read name/ordinal pairs
                name_offset = rva_to_offset(data.sections, names_rva)
                ord_offset = rva_to_offset(data.sections, ordinals_rva)
                if name_offset and ord_offset:
                    for i in range(num_names):
                        f.seek(name_offset + i * 4)
                        fn_name_rva = struct.unpack("<I", f.read(4))[0]
                        f.seek(ord_offset + i * 2)
                        ordinal_idx = struct.unpack("<H", f.read(2))[0]

                        fn_name = ""
                        fn_name_off = rva_to_offset(data.sections, fn_name_rva)
                        if fn_name_off:
                            fn_name = read_cstring(f, fn_name_off)

                        forwarded = False
                        fwd_name = ""
                        if ordinal_idx < len(func_rvas):
                            fwd_rva = func_rvas[ordinal_idx]
                            if export_rva <= fwd_rva < export_rva + export_size:
                                forwarded = True
                                fwd_off = rva_to_offset(data.sections, fwd_rva)
                                if fwd_off:
                                    fwd_name = read_cstring(f, fwd_off)

                        data.exports.append(PEExport(
                            name=fn_name,
                            ordinal=ordinal_base + ordinal_idx,
                            rva=func_rvas[ordinal_idx] if ordinal_idx < len(func_rvas) else 0,
                            forwarded=forwarded,
                            forward_name=fwd_name,
                        ))

                # Ordinal-only exports
                for idx in range(num_functions):
                    if not any(e.ordinal == ordinal_base + idx for e in data.exports):
                        data.exports.append(PEExport(
                            name=f"Ordinal_{ordinal_base + idx}",
                            ordinal=ordinal_base + idx,
                            rva=func_rvas[idx] if idx < len(func_rvas) else 0,
                        ))

        # Parse Imports
        for is_delay in [False, True]:
            dir_idx = IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT if is_delay else IMAGE_DIRECTORY_ENTRY_IMPORT
            imp_rva, imp_size = data_dirs[dir_idx]
            if imp_rva <= 0:
                continue

            imp_offset = rva_to_offset(data.sections, imp_rva)
            if not imp_offset:
                continue

            f.seek(imp_offset)
            while True:
                if is_delay:
                    # Delay import descriptor
                    attrs = struct.unpack("<I", f.read(4))[0]
                    name_rva = struct.unpack("<I", f.read(4))[0]
                    module_handle_rva = struct.unpack("<I", f.read(4))[0]
                    iat_rva = struct.unpack("<I", f.read(4))[0]
                    bound_iat_rva = struct.unpack("<I", f.read(4))[0]
                    unload_iat_rva = struct.unpack("<I", f.read(4))[0]
                    timestamp = struct.unpack("<I", f.read(4))[0]

                    if name_rva == 0:
                        break

                    dll_name_off = rva_to_offset(data.sections, name_rva)
                    if not dll_name_off:
                        break
                    dll_name = read_cstring(f, dll_name_off)

                    funcs = []
                    if iat_rva:
                        iat_off = rva_to_offset(data.sections, iat_rva)
                        if iat_off:
                            f_pos = f.tell()
                            f.seek(iat_off)
                            while True:
                                if is_pe32plus:
                                    entry = struct.unpack("<Q", f.read(8))[0]
                                else:
                                    entry = struct.unpack("<I", f.read(4))[0]
                                if entry == 0:
                                    break
                                if entry & (1 << (63 if is_pe32plus else 31)):
                                    ordinal_num = entry & 0xFFFF
                                    funcs.append(f"Ordinal({ordinal_num})")
                                else:
                                    hint_rva = rva_to_offset(data.sections, entry & 0x7FFFFFFF)
                                    if hint_rva:
                                        f.seek(hint_rva + 2)
                                        fn = read_cstring(f)
                                        funcs.append(fn)
                                    else:
                                        funcs.append(f"RVA_0x{entry:X}")
                            f.seek(f_pos)

                    imp = PEImport(dll_name=dll_name, functions=funcs, is_delay=True)
                    data.delay_imports.append(imp)
                else:
                    # Standard import descriptor
                    _ = struct.unpack("<I", f.read(4))[0]  # OriginalFirstThunk
                    _ = struct.unpack("<I", f.read(4))[0]  # TimeDateStamp
                    _ = struct.unpack("<H", f.read(2))[0]  # ForwarderChain
                    _ = struct.unpack("<H", f.read(2))[0]  # MajorVersion
                    _ = struct.unpack("<H", f.read(2))[0]  # MinorVersion
                    dll_name_rva = struct.unpack("<I", f.read(4))[0]
                    iat_rva = struct.unpack("<I", f.read(4))[0]

                    if dll_name_rva == 0:
                        break

                    dll_name_off = rva_to_offset(data.sections, dll_name_rva)
                    if not dll_name_off:
                        break
                    dll_name = read_cstring(f, dll_name_off)

                    funcs = []
                    if iat_rva:
                        iat_off = rva_to_offset(data.sections, iat_rva)
                        if iat_off:
                            f_pos = f.tell()
                            f.seek(iat_off)
                            while True:
                                if is_pe32plus:
                                    entry = struct.unpack("<Q", f.read(8))[0]
                                else:
                                    entry = struct.unpack("<I", f.read(4))[0]
                                if entry == 0:
                                    break
                                if entry & (1 << (63 if is_pe32plus else 31)):
                                    ordinal_num = entry & 0xFFFF
                                    funcs.append(f"Ordinal({ordinal_num})")
                                else:
                                    hint_rva = rva_to_offset(data.sections, entry & 0x7FFFFFFF)
                                    if hint_rva:
                                        f.seek(hint_rva + 2)
                                        fn = read_cstring(f)
                                        funcs.append(fn)
                                    else:
                                        funcs.append(f"RVA_0x{entry:X}")
                            f.seek(f_pos)

                    imp = PEImport(dll_name=dll_name, functions=funcs)
                    data.imports.append(imp)

        # Detect anomalies
        ts = datetime(1970, 1, 1) + timedelta(seconds=data.timestamp)
        if ts.year < 2010 or ts.year > 2015:
            data.anomalies.append(f"Unusual timestamp: {ts.isoformat()}")

        if data.entry_point_rva == 0:
            data.anomalies.append("Entry point RVA is 0 (DLL or no entry)")

        for sec in data.sections:
            if sec.entropy > 7.5:
                data.anomalies.append(f"High entropy section '{sec.name}': {sec.entropy} (possible packing/encryption)")
            if sec.name and not sec.name.startswith("."):
                data.anomalies.append(f"Non-standard section name: '{sec.name}'")

        if not data.exports:
            data.anomalies.append("No exports found")
        if not data.imports:
            data.anomalies.append("No imports found (statically linked or packed?)")

        return data


def rva_to_offset(sections: List[PESection], rva: int) -> Optional[int]:
    for sec in sections:
        if sec.virtual_address <= rva < sec.virtual_address + sec.virtual_size:
            return rva - sec.virtual_address + sec.raw_offset
    return None


def format_size(size: int) -> str:
    if size >= 1024 * 1024:
        return f"{size / (1024*1024):.1f} MB"
    elif size >= 1024:
        return f"{size / 1024:.1f} KB"
    return f"{size} B"


def print_pe_report(data: PEData):
    print(f"\n{'='*70}")
    print(f"  PE Analysis: {data.filename}")
    print(f"{'='*70}")

    ts = datetime(1970, 1, 1) + timedelta(seconds=data.timestamp) if data.timestamp else None
    print(f"\n  File:           {data.filepath}")
    print(f"  Size:           {format_size(data.file_size)}")
    print(f"  Architecture:   {'x64 (PE32+)' if data.is_64bit else 'x86 (PE32)'}")
    print(f"  Machine:        0x{data.machine:04X}")
    print(f"  Image Base:     0x{data.image_base:X}")
    print(f"  Entry Point:    0x{data.entry_point_rva:X}")
    print(f"  Timestamp:      {ts.isoformat() if ts else 'N/A'}")

    print(f"\n  --- Sections ({len(data.sections)}) ---")
    print(f"  {'Name':<10} {'VirtAddr':>10} {'VirtSize':>10} {'RawSize':>10} {'Entropy':>8} {'Flags'}")
    for sec in data.sections:
        flags = ""
        if sec.is_code: flags += "CODE "
        if sec.is_executable: flags += "EXEC "
        if sec.is_readable: flags += "R "
        if sec.is_writable: flags += "W "
        print(f"  {sec.name:<10} 0x{sec.virtual_address:08X} {sec.virtual_size:>10} {sec.raw_size:>10} {sec.entropy:>8.2f} {flags}")

    print(f"\n  --- Exports ({len(data.exports)}) ---")
    for exp in sorted(data.exports, key=lambda e: e.ordinal):
        fwd = f" -> {exp.forward_name}" if exp.forwarded else ""
        print(f"    #{exp.ordinal:<5} {exp.name}{fwd}")

    print(f"\n  --- Imports ({len(data.imports)} DLLs) ---")
    for imp in data.imports:
        print(f"    {imp.dll_name} ({len(imp.functions)} functions)")
        for fn in imp.functions[:5]:
            print(f"      - {fn}")
        if len(imp.functions) > 5:
            print(f"      ... and {len(imp.functions) - 5} more")

    if data.delay_imports:
        print(f"\n  --- Delay Imports ({len(data.delay_imports)} DLLs) ---")
        for imp in data.delay_imports:
            print(f"    {imp.dll_name} ({len(imp.functions)} functions)")

    if data.anomalies:
        print(f"\n  --- Anomalies ({len(data.anomalies)}) ---")
        for a in data.anomalies:
            print(f"    ! {a}")

    print()


def compare_pes(orig: PEData, recon: PEData):
    print(f"\n{'='*70}")
    print(f"  PE Comparison: {orig.filename} vs {recon.filename}")
    print(f"{'='*70}")

    # Export comparison
    orig_names = {e.name for e in orig.exports if not e.forwarded}
    recon_names = {e.name for e in recon.exports if not e.forwarded}

    missing = orig_names - recon_names
    extra = recon_names - orig_names
    common = orig_names & recon_names

    print(f"\n  Exports:")
    print(f"    Original: {len(orig_names)} | Recreation: {len(recon_names)} | Common: {len(common)}")
    if missing:
        print(f"    Missing in recreation ({len(missing)}):")
        for n in sorted(missing)[:20]:
            print(f"      - {n}")
        if len(missing) > 20:
            print(f"      ... and {len(missing) - 20} more")
    if extra:
        print(f"    Extra in recreation ({len(extra)}):")
        for n in sorted(extra)[:20]:
            print(f"      - {n}")

    # Import comparison
    orig_dlls = {i.dll_name.lower() for i in orig.imports}
    recon_dlls = {i.dll_name.lower() for i in recon.imports}

    print(f"\n  Imports:")
    print(f"    Original DLLs: {len(orig_dlls)} | Recreation DLLs: {len(recon_dlls)}")
    missing_dlls = orig_dlls - recon_dlls
    extra_dlls = recon_dlls - orig_dlls
    if missing_dlls:
        print(f"    Missing: {', '.join(sorted(missing_dlls))}")
    if extra_dlls:
        print(f"    Extra: {', '.join(sorted(extra_dlls))}")

    # Section comparison
    print(f"\n  Sections:")
    print(f"    Original: {len(orig.sections)} | Recreation: {len(recon.sections)}")
    for o_sec in orig.sections:
        r_sec = next((s for s in recon.sections if s.name == o_sec.name), None)
        if r_sec:
            size_diff = r_sec.raw_size - o_sec.raw_size
            pct = (size_diff / o_sec.raw_size * 100) if o_sec.raw_size else 0
            print(f"    {o_sec.name:<10} Orig: {format_size(o_sec.raw_size):>10} Recon: {format_size(r_sec.raw_size):>10} ({pct:+.1f}%)")
        else:
            print(f"    {o_sec.name:<10} Orig: {format_size(o_sec.raw_size):>10} Recon: MISSING")

    print()


def batch_analyze(directory: str):
    results = []
    for ext in ["*.exe", "*.dll"]:
        for filepath in Path(directory).glob(ext):
            try:
                data = parse_pe(str(filepath))
                results.append(data)
            except Exception as e:
                print(f"  Error analyzing {filepath}: {e}")

    for data in sorted(results, key=lambda d: d.filename):
        print_pe_report(data)

    # Save JSON
    report_path = os.path.join(directory, "..", "tools", "reports", "pe_batch_report.json")
    os.makedirs(os.path.dirname(report_path), exist_ok=True)
    with open(report_path, "w") as f:
        json.dump([d.to_dict() for d in results], f, indent=2, default=str)
    print(f"\n  Batch report saved to {report_path}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    if sys.argv[1] == "--compare" and len(sys.argv) >= 4:
        orig = parse_pe(sys.argv[2])
        recon = parse_pe(sys.argv[3])
        print_pe_report(orig)
        print_pe_report(recon)
        compare_pes(orig, recon)
    elif sys.argv[1] == "--batch" and len(sys.argv) >= 3:
        batch_analyze(sys.argv[2])
    else:
        data = parse_pe(sys.argv[1])
        print_pe_report(data)

        # Save JSON report
        report_path = os.path.join("tools", "reports", f"{data.filename}_pe.json")
        os.makedirs(os.path.dirname(report_path), exist_ok=True)
        with open(report_path, "w") as f:
            json.dump(data.to_dict(), f, indent=2, default=str)
        print(f"  Report saved to {report_path}")
