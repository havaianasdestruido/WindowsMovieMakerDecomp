#!/usr/bin/env python3
"""
Exhaustive PE Analysis of MovieMakerCore.dll
Windows Live Essentials 2012 - Movie Maker Core Module
"""

import pefile
import os
import sys
import struct
import io
from collections import defaultdict

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

try:
    from capstone import *
    HAS_CAPSTONE = True
except ImportError:
    HAS_CAPSTONE = False
    print("[!] capstone not installed, disassembly sections will be skipped")

DLL_PATH = r"C:\Users\mcmco\Desktop\WMMR\undecomp\Photo Gallery\MovieMakerCore.dll"

SEP = "=" * 80
SUBSEP = "-" * 80

def hr(title):
    print(f"\n{SEP}")
    print(f"  {title}")
    print(SEP)

def sub_hr(title):
    print(f"\n{SUBSEP}")
    print(f"  {title}")
    print(SUBSEP)

def dump_pe_headers(pe):
    hr("1. FULL PE HEADER DUMP")
    print(f"  File: {DLL_PATH}")
    print(f"  File Size: {os.path.getsize(DLL_PATH):,} bytes")
    print()
    print("  --- DOS Header ---")
    dos = pe.DOS_HEADER
    fields = [
        ("e_magic", dos.e_magic),
        ("e_cblp", dos.e_cblp),
        ("e_cp", dos.e_cp),
        ("e_crlc", dos.e_crlc),
        ("e_cparhdr", dos.e_cparhdr),
        ("e_minalloc", dos.e_minalloc),
        ("e_maxalloc", dos.e_maxalloc),
        ("e_ss", dos.e_ss),
        ("e_sp", dos.e_sp),
        ("e_csum", dos.e_csum),
        ("e_ip", dos.e_ip),
        ("e_cs", dos.e_cs),
        ("e_lfarlc", dos.e_lfarlc),
        ("e_ovno", dos.e_ovno),
        ("e_oemid", dos.e_oemid),
        ("e_oeminfo", dos.e_oeminfo),
        ("e_lfanew", dos.e_lfanew),
    ]
    for name, val in fields:
        print(f"    {name:20s}: 0x{val:04X} ({val})")

    print()
    print("  --- NT Signature ---")
    print(f"    Signature: 0x{pe.NT_HEADERS.Signature:08X}")
    if hasattr(pe.NT_HEADERS.Signature, 'Signature'):
        print(f"    PE\\0\\0 verified: {pe.NT_HEADERS.Signature.Signature == 0x00004550}")

    print()
    print("  --- File Header ---")
    fh = pe.FILE_HEADER
    print(f"    Machine:              0x{fh.Machine:04X}", end="")
    machine_map = {0x0: "Unknown", 0x14c: "x86 (I386)", 0x8664: "x64 (AMD64)", 0x1c0: "ARM", 0xaa64: "ARM64", 0x1c4: "ARM Thumb-2"}
    print(f"  ({machine_map.get(fh.Machine, 'Unknown')})")
    print(f"    NumberOfSections:     {fh.NumberOfSections}")
    print(f"    TimeDateStamp:        0x{fh.TimeDateStamp:08X}", end="")
    if fh.TimeDateStamp:
        import datetime
        try:
            ts = datetime.datetime.utcfromtimestamp(fh.TimeDateStamp)
            print(f"  ({ts})")
        except:
            print()
    else:
        print()
    print(f"    PointerToSymbolTable: 0x{fh.PointerToSymbolTable:08X}")
    print(f"    NumberOfSymbols:      {fh.NumberOfSymbols}")
    print(f"    SizeOfOptionalHeader: {fh.SizeOfOptionalHeader}")
    print(f"    Characteristics:      0x{fh.Characteristics:04X}")
    chars = []
    char_flags = {
        0x0001: "RELOCS_STRIPPED", 0x0002: "EXECUTABLE_IMAGE",
        0x0004: "LINE_NUMS_STRIPPED", 0x0008: "LOCAL_SYMS_STRIPPED",
        0x0020: "LARGE_ADDRESS_AWARE", 0x0080: "BYTES_REVERSED_LO",
        0x0100: "32BIT_MACHINE", 0x0200: "DEBUG_STRIPPED",
        0x0400: "REMOVABLE_RUN_FROM_SWAP", 0x0800: "NET_RUN_FROM_SWAP",
        0x1000: "SYSTEM", 0x2000: "DLL", 0x4000: "UP_SYSTEM_ONLY",
        0x8000: "BYTES_REVERSED_HI",
    }
    for bit, name in char_flags.items():
        if fh.Characteristics & bit:
            chars.append(name)
    print(f"    Characteristics Flags: [{', '.join(chars)}]")

    print()
    print("  --- Optional Header ---")
    oh = pe.OPTIONAL_HEADER
    print(f"    Magic:                   0x{oh.Magic:04X}", end="")
    magic_map = {0x10b: "PE32", 0x20b: "PE32+ (PE64)", 0x107: "ROM"}
    print(f"  ({magic_map.get(oh.Magic, 'Unknown')})")
    print(f"    MajorLinkerVersion:      {oh.MajorLinkerVersion}")
    print(f"    MinorLinkerVersion:      {oh.MinorLinkerVersion}")
    print(f"    SizeOfCode:              0x{oh.SizeOfCode:08X} ({oh.SizeOfCode:,})")
    print(f"    SizeOfInitializedData:   0x{oh.SizeOfInitializedData:08X}")
    print(f"    SizeOfUninitializedData: 0x{oh.SizeOfUninitializedData:08X}")
    print(f"    AddressOfEntryPoint:     0x{oh.AddressOfEntryPoint:08X}")
    print(f"    BaseOfCode:              0x{oh.BaseOfCode:08X}")
    if oh.Magic == 0x10b:
        print(f"    BaseOfData:              0x{oh.BaseOfData:08X}")
    if oh.Magic == 0x10b:
        print(f"    ImageBase:               0x{oh.ImageBase:08X}")
    else:
        print(f"    ImageBase:               0x{oh.ImageBase:016X}")
    print(f"    SectionAlignment:        0x{oh.SectionAlignment:08X}")
    print(f"    FileAlignment:           0x{oh.FileAlignment:08X}")
    print(f"    MajorOperatingSystemVer: {oh.MajorOperatingSystemVersion}")
    print(f"    MinorOperatingSystemVer: {oh.MinorOperatingSystemVersion}")
    print(f"    MajorImageVersion:       {oh.MajorImageVersion}")
    print(f"    MinorImageVersion:       {oh.MinorImageVersion}")
    print(f"    MajorSubsystemVersion:   {oh.MajorSubsystemVersion}")
    print(f"    MinorSubsystemVersion:   {oh.MinorSubsystemVersion}")
    if oh.Magic == 0x10b:
        if hasattr(oh, 'Win32VersionValue'):
            print(f"    Win32VersionValue:       0x{oh.Win32VersionValue:08X}")
    print(f"    SizeOfImage:             0x{oh.SizeOfImage:08X} ({oh.SizeOfImage:,})")
    print(f"    SizeOfHeaders:           0x{oh.SizeOfHeaders:08X}")
    print(f"    CheckSum:                0x{oh.CheckSum:08X}")
    print(f"    Subsystem:               0x{oh.Subsystem:04X}", end="")
    sub_map = {1: "NATIVE", 2: "WINDOWS_GUI", 3: "WINDOWS_CUI",
               5: "OS2_CUI", 7: "POSIX_CUI", 9: "WINDOWS_CE_GUI",
               10: "EFI_APPLICATION", 14: "WINDOWS_BOOT_APPLICATION"}
    print(f"  ({sub_map.get(oh.Subsystem, 'Unknown')})")
    print(f"    DllCharacteristics:      0x{oh.DllCharacteristics:04X}")
    dll_chars = []
    dll_char_flags = {
        0x0020: "HIGH_ENTROPY_VA", 0x0040: "DYNAMIC_BASE (ASLR)",
        0x0080: "FORCE_INTEGRITY_CHECKS", 0x0100: "NX_COMPAT (DEP)",
        0x0200: "NO_ISOLATION", 0x0400: "NO_SEH",
        0x0800: "NO_BIND", 0x1000: "APPCONTAINER",
        0x2000: "WDM_DRIVER", 0x4000: "GUARD_CF",
        0x8000: "TERMINAL_SERVER_AWARE",
    }
    for bit, name in dll_char_flags.items():
        if oh.DllCharacteristics & bit:
            dll_chars.append(name)
    print(f"    DllCharacteristics Flags: [{', '.join(dll_chars)}]")
    if oh.Magic == 0x10b:
        print(f"    SizeOfStackReserve:      0x{oh.SizeOfStackReserve:08X}")
        print(f"    SizeOfStackCommit:       0x{oh.SizeOfStackCommit:08X}")
        print(f"    SizeOfHeapReserve:       0x{oh.SizeOfHeapReserve:08X}")
        print(f"    SizeOfHeapCommit:        0x{oh.SizeOfHeapCommit:08X}")
    else:
        print(f"    SizeOfStackReserve:      0x{oh.SizeOfStackReserve:016X}")
        print(f"    SizeOfStackCommit:       0x{oh.SizeOfStackCommit:016X}")
        print(f"    SizeOfHeapReserve:       0x{oh.SizeOfHeapReserve:016X}")
        print(f"    SizeOfHeapCommit:        0x{oh.SizeOfHeapCommit:016X}")
    print(f"    LoaderFlags:             0x{oh.LoaderFlags:08X}")
    print(f"    NumberOfRvaAndSizes:     {oh.NumberOfRvaAndSizes}")

    print()
    print("  --- Data Directories ---")
    dir_names = [
        "EXPORT", "IMPORT", "RESOURCE", "EXCEPTION", "SECURITY",
        "BASERELOC", "DEBUG", "ARCHITECTURE", "GLOBALPTR", "TLS",
        "LOAD_CONFIG", "BOUND_IMPORT", "IAT", "DELAY_IMPORT",
        "COM_DESCRIPTOR", "Reserved"
    ]
    for i, entry in enumerate(oh.DATA_DIRECTORY):
        name = dir_names[i] if i < len(dir_names) else f"DIR_{i}"
        if entry.VirtualAddress and entry.Size:
            print(f"    [{i:2d}] {name:20s}: RVA=0x{entry.VirtualAddress:08X}  Size=0x{entry.Size:08X}")
        elif entry.VirtualAddress or entry.Size:
            print(f"    [{i:2d}] {name:20s}: RVA=0x{entry.VirtualAddress:08X}  Size=0x{entry.Size:08X}  (PARTIAL)")
        else:
            print(f"    [{i:2d}] {name:20s}: (empty)")

    print()
    print("  --- Delay Load Import Directories ---")
    try:
        for entry in pe.OPTIONAL_HEADER.DATA_DIRECTORY:
            if hasattr(entry, 'imports') and entry.imports:
                pass
    except:
        pass


def dump_sections(pe):
    hr("2. ALL SECTIONS WITH FULL DETAILS")
    print(f"  Total sections: {len(pe.sections)}")
    print()
    for i, sec in enumerate(pe.sections):
        name = sec.Name.rstrip(b'\x00').decode('ascii', errors='replace')
        print(f"  [{i}] \"{name}\"")
        print(f"      VirtualSize:      0x{sec.Misc_VirtualSize:08X} ({sec.Misc_VirtualSize:,})")
        print(f"      VirtualAddress:   0x{sec.VirtualAddress:08X}")
        print(f"      SizeOfRawData:    0x{sec.SizeOfRawData:08X} ({sec.SizeOfRawData:,})")
        print(f"      PointerToRawData: 0x{sec.PointerToRawData:08X}")
        print(f"      PointerToRelocs:  0x{sec.PointerToRelocations:08X}")
        print(f"      PointerToLinNums: 0x{sec.PointerToLinenumbers:08X}")
        print(f"      NumberOfRelocs:   {sec.NumberOfRelocations}")
        print(f"      NumberOfLinNums:  {sec.NumberOfLinenumbers}")
        print(f"      Characteristics:  0x{sec.Characteristics:08X}")
        sec_chars = []
        sec_char_flags = {
            0x00000020: "CODE", 0x00000040: "INITIALIZED_DATA",
            0x00000080: "UNINITIALIZED_DATA", 0x00000100: "LNK_OTHER",
            0x00000200: "LNK_INFO", 0x00000800: "LNK_REMOVE",
            0x00001000: "LNK_COMDAT", 0x00002000: "NO_DEFER_SPEC_EXC",
            0x00004000: "GPREL", 0x00100000: "LNK_NRELOC_OVFL",
            0x00200000: "DISCARDABLE", 0x00400000: "NOT_CACHED",
            0x00800000: "NOT_PAGED", 0x01000000: "SHARED",
            0x02000000: "EXECUTE", 0x04000000: "READ",
            0x08000000: "WRITE",
        }
        for bit, cname in sec_char_flags.items():
            if sec.Characteristics & bit:
                sec_chars.append(cname)
        print(f"      Flags:            [{', '.join(sec_chars)}]")
        entropy = sec.get_entropy()
        print(f"      Entropy:          {entropy:.4f}", end="")
        if entropy > 7.0:
            print(" (HIGH - likely packed/encrypted)")
        elif entropy > 6.0:
            print(" (MEDIUM - compiled code)")
        else:
            print(" (LOW - data/strings)")
        print()


def dump_imports(pe):
    hr("3. ALL IMPORTS ORGANIZED BY DLL")
    if not pe.DIRECTORY_ENTRY_IMPORT:
        print("  No imports found!")
        return

    total_funcs = 0
    total_dlls = 0
    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        total_dlls += 1
        dll_name = entry.dll.decode('utf-8', errors='replace')
        funcs = entry.imports
        total_funcs += len(funcs)

    print(f"  Total imported DLLs: {total_dlls}")
    print(f"  Total imported functions: {total_funcs}")
    print()

    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        dll_name = entry.dll.decode('utf-8', errors='replace')
        print(f"  --- {dll_name} ---")
        for func in entry.imports:
            if func.name:
                fname = func.name.decode('utf-8', errors='replace')
                print(f"    {fname}  (RVA: 0x{func.address:08X})")
            else:
                print(f"    ordinal #{func.ordinal}  (RVA: 0x{func.address:08X})")
        print()


def dump_exports(pe):
    hr("4. ALL EXPORTS (CRITICAL - CORE API)")
    if not pe.DIRECTORY_ENTRY_EXPORT:
        print("  No exports found!")
        return

    exp = pe.DIRECTORY_ENTRY_EXPORT
    print(f"  Export Directory RVA: 0x{exp.struct.Characteristics:08X}")
    print(f"  TimeDateStamp:        0x{exp.struct.TimeDateStamp:08X}")
    print(f"  MajorVersion:         {exp.struct.MajorVersion}")
    print(f"  MinorVersion:         {exp.struct.MinorVersion}")
    print(f"  Name RVA:             0x{exp.struct.Name:08X}")
    print(f"  Ordinal Base:         {exp.struct.Base}")
    print(f"  NumberOfFunctions:    {exp.struct.NumberOfFunctions}")
    print(f"  NumberOfNames:        {exp.struct.NumberOfNames}")
    print()

    if exp.name:
        print(f"  DLL Name: {exp.name.decode('utf-8', errors='replace')}")
    print()

    if exp.symbols:
        print(f"  Exported Symbols: {len(exp.symbols)}")
        print()
        named_exports = []
        ordinal_only = []
        for sym in exp.symbols:
            if sym.name:
                named_exports.append(sym)
            else:
                ordinal_only.append(sym)

        print(f"  Named exports: {len(named_exports)}")
        print(f"  Ordinal-only exports: {len(ordinal_only)}")
        print()

        print("  --- Named Exports (sorted by name) ---")
        print(f"  {'Ordinal':>8s}  {'RVA':>12s}  Name")
        print(f"  {'-'*8}  {'-'*12}  {'-'*40}")
        for sym in sorted(named_exports, key=lambda s: s.name.decode('utf-8', errors='replace') if s.name else ""):
            name = sym.name.decode('utf-8', errors='replace') if sym.name else "(none)"
            hint_str = ""
            if hasattr(sym, 'hint') and sym.hint is not None:
                hint_str = f"  Hint: {sym.hint}"
            print(f"  {sym.ordinal:8d}  0x{sym.address:08X}  {name}{hint_str}")

        if ordinal_only:
            print()
            print("  --- Ordinal-Only Exports ---")
            print(f"  {'Ordinal':>8s}  {'RVA':>12s}")
            print(f"  {'-'*8}  {'-'*12}")
            for sym in ordinal_only:
                print(f"  {sym.ordinal:8d}  0x{sym.address:08X}")

        print()
        print("  --- Exported Names Table (sorted alphabetically) ---")
        if hasattr(exp, 'export_name_table') and exp.export_name_table:
            for idx, (name_bytes, sym) in enumerate(sorted(exp.export_name_table.items())):
                name = name_bytes.decode('utf-8', errors='replace') if name_bytes else f"ordinal#{sym.ordinal}"
                print(f"    {idx+1:5d}. {name}")
        elif hasattr(exp, 'name_list') and exp.name_list:
            for idx, name_entry in enumerate(exp.name_list):
                name = name_entry.name.decode('utf-8', errors='replace') if name_entry.name else f"ordinal#{name_entry.ordinal}"
                print(f"    {idx+1:5d}. {name}")


def dump_rtti_and_classes(pe):
    hr("5. CLASS NAMES, RTTI PATTERNS, AND VTABLE REFERENCES")
    print("  Scanning all sections for MSVC RTTI patterns and class names...")
    print()

    rtti_patterns = defaultdict(list)
    class_names = []

    rtti_markers = [
        b'?AV', b'?UA', b'?AC', b'?AW', b'?AU',
        b'?$AV', b'?$AA', b'?$AU', b'?$AC',
    ]

    for sec in pe.sections:
        if sec.SizeOfRawData == 0:
            continue
        data = sec.get_data()
        section_name = sec.Name.rstrip(b'\x00').decode('ascii', errors='replace')

        for marker in rtti_markers:
            offset = 0
            while True:
                pos = data.find(marker, offset)
                if pos == -1:
                    break
                end = pos
                while end < len(data) and end < pos + 200:
                    if data[end] in (0, 0xFF):
                        break
                    end += 1
                try:
                    name_str = data[pos:end].decode('utf-8', errors='replace')
                    rva = sec.VirtualAddress + pos
                    rtti_patterns[marker.decode('ascii', errors='replace')].append((name_str, section_name, rva))
                except:
                    pass
                offset = pos + 1

    for marker_name, entries in sorted(rtti_patterns.items()):
        print(f"  --- Pattern: \"{marker_name}\" ({len(entries)} matches) ---")
        seen = set()
        for name, sec_name, rva in entries:
            if name not in seen:
                seen.add(name)
                try:
                    print(f"    {name}  [{sec_name}+0x{rva:X}]")
                except UnicodeEncodeError:
                    safe = name.encode('ascii', errors='replace').decode('ascii')
                    print(f"    {safe}  [{sec_name}+0x{rva:X}]")
        if len(seen) > 100:
            print(f"    ... ({len(seen)} unique entries, showing first 100)")
        print()

    vtable_count = 0
    print("  --- Vtable Pointer Pattern Scan ---")
    for sec in pe.sections:
        if sec.SizeOfRawData == 0:
            continue
        data = sec.get_data()
        section_name = sec.Name.rstrip(b'\x00').decode('ascii', errors='replace')
        if not data:
            continue
        img_base = pe.OPTIONAL_HEADER.ImageBase
        code_start = sec.VirtualAddress + img_base
        code_end = sec.VirtualAddress + img_base + sec.Misc_VirtualSize
        for i in range(0, min(len(data) - 4, 50000000), 4):
            val = struct.unpack_from('<I', data, i)[0]
            if code_start <= val <= code_end:
                vtable_count += 1

    print(f"  Potential vtable references in code sections: ~{vtable_count}")
    print()


def categorize_string(s):
    s_lower = s.lower()
    if any(x in s_lower for x in ['c:\\', 'd:\\', 'e:\\', 'windows\\', 'program files', 'appdata', 'temp\\', '.dll', '.exe', '.xml', '.ini', '.tmp', '.dat']):
        return "File Path"
    if any(x in s_lower for x in ['http://', 'https://', 'ftp://', '.com/', '.net/', '.org/', 'www.']):
        return "URL"
    if any(x in s_lower for x in ['hkey_', 'registry', 'software\\', 'microsoft\\']):
        return "Registry Key"
    if any(x in s_lower for x in ['error', 'fail', 'invalid', 'cannot', 'unable', 'exception', 'warning', 'fault']):
        return "Error Message"
    if any(x in s_lower for x in ['button', 'ok', 'cancel', 'dialog', 'window', 'title', 'text', 'label', 'menu', 'file', 'edit', 'save', 'open']):
        return "UI String"
    if any(x in s_lower for x in ['{000', '{1', 'iid', 'clsid', 'iunknown', 'idispatch', 'iclass']):
        return "COM Interface"
    if any(x in s_lower for x in ['class ', 'struct ', 'namespace ', 'void ', 'int ', 'bool ', 'string', 'bool', ' HRESULT', 'stdcall', 'cdecl']):
        return "Function/Class"
    if 'movie' in s_lower or 'video' in s_lower or 'photo' in s_lower or 'transition' in s_lower or 'effect' in s_lower or 'timeline' in s_lower:
        return "MovieMaker Domain"
    return "Other"


def dump_strings(pe):
    hr("6. ALL MEANINGFUL STRINGS (min length 4)")
    min_len = 4
    all_strings = []
    string_categories = defaultdict(list)

    for sec in pe.sections:
        if sec.SizeOfRawData == 0:
            continue
        data = sec.get_data()
        section_name = sec.Name.rstrip(b'\x00').decode('ascii', errors='replace')
        if not data:
            continue

        current = []
        for byte in data:
            if 32 <= byte < 127:
                current.append(chr(byte))
            else:
                if len(current) >= min_len:
                    s = ''.join(current)
                    rva = sec.VirtualAddress
                    cat = categorize_string(s)
                    string_categories[cat].append((s, section_name, rva))
                current = []

    total = sum(len(v) for v in string_categories.values())
    print(f"  Total meaningful strings found: {total}")
    print()

    for cat in sorted(string_categories.keys()):
        strings = string_categories[cat]
        print(f"  --- Category: {cat} ({len(strings)} strings) ---")
        for s, sec_name, rva in strings[:500]:
            print(f"    [{sec_name}] {s[:120]}")
        if len(strings) > 500:
            print(f"    ... ({len(strings)} total, showing first 500)")
        print()


def dump_resources(pe):
    hr("7. RESOURCE TREE DUMP")
    if not hasattr(pe, 'DIRECTORY_ENTRY_RESOURCE') or not pe.DIRECTORY_ENTRY_RESOURCE:
        print("  No resources found!")
        return

    rt_types = {
        1: "RT_CURSOR", 2: "RT_BITMAP", 3: "RT_ICON", 4: "RT_MENU",
        5: "RT_DIALOG", 6: "RT_STRING", 7: "RT_FONTDIR", 8: "RT_FONT",
        9: "RT_ACCELERATOR", 10: "RT_RCDATA", 11: "RT_MESSAGETABLE",
        12: "RT_GROUP_CURSOR", 14: "RT_GROUP_ICON", 16: "RT_VERSION",
        17: "RT_DLGINCLUDE", 19: "RT_PLUGPLAY", 20: "RT_VXD",
        21: "RT_ANICURSOR", 22: "RT_ANIICON", 23: "RT_HTML",
        24: "RT_MANIFEST",
    }

    def print_resource_entry(entry, indent=4):
        spaces = " " * indent
        if hasattr(entry, 'struct'):
            s = entry.struct
            rid = s.Id if hasattr(s, 'Id') else 0
            name_id = s.Name if hasattr(s, 'Name') else 0
            type_str = ""
            if rid in rt_types:
                type_str = f" ({rt_types[rid]})"
            print(f"{spaces}Resource ID: 0x{rid:04X}{type_str}")
            if hasattr(s, 'TimeDateStamp') and s.TimeDateStamp:
                print(f"{spaces}  TimeDateStamp:  0x{s.TimeDateStamp:08X}")
            if hasattr(s, 'NumberOfNamedEntries') and hasattr(s, 'NumberOfIdEntries'):
                print(f"{spaces}  NamedEntries:   {s.NumberOfNamedEntries}")
                print(f"{spaces}  IdEntries:      {s.NumberOfIdEntries}")

        if hasattr(entry, 'directory') and entry.directory:
            for sub in entry.directory.entries:
                print_resource_entry(sub, indent + 4)
        elif hasattr(entry, 'data') and entry.data:
            d = entry.data
            print(f"{spaces}  --- Data Entry ---")
            if hasattr(d, 'struct'):
                ds = d.struct
                if hasattr(ds, 'OffsetToData'):
                    print(f"{spaces}    OffsetToData: 0x{ds.OffsetToData:08X}")
                if hasattr(ds, 'Size'):
                    print(f"{spaces}    Size:         0x{ds.Size:08X} ({ds.Size:,} bytes)")
                if hasattr(ds, 'CodePage'):
                    print(f"{spaces}    CodePage:     {ds.CodePage}")

    for entry in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        print_resource_entry(entry)
        print()


def disasm_entry_point(pe):
    hr("8. DISASSEMBLY OF DLLMAIN ENTRY POINT (first 100 instructions)")
    if not HAS_CAPSTONE:
        print("  [Skipped - capstone not installed]")
        return

    ep_rva = pe.OPTIONAL_HEADER.AddressOfEntryPoint
    print(f"  Entry Point RVA: 0x{ep_rva:08X}")
    print(f"  ImageBase: 0x{pe.OPTIONAL_HEADER.ImageBase:08X}")
    print()

    ep_data = None
    for sec in pe.sections:
        if sec.VirtualAddress <= ep_rva < sec.VirtualAddress + sec.SizeOfRawData:
            offset = ep_rva - sec.VirtualAddress
            ep_data = sec.get_data()
            ep_data = ep_data[offset:offset + 512]
            break

    if not ep_data:
        print("  Could not locate entry point bytes!")
        return

    mode = CS_MODE_32 if pe.FILE_HEADER.Machine == 0x14c else CS_MODE_64
    md = Cs(CS_ARCH_X86, mode)
    md.detail = True

    img_base = pe.OPTIONAL_HEADER.ImageBase
    print(f"  --- Disassembly at 0x{img_base + ep_rva:08X} ---")
    count = 0
    for insn in md.disasm(ep_data, img_base + ep_rva):
        print(f"    0x{insn.address:08X}:  {insn.mnemonic:8s} {insn.op_str}")
        count += 1
        if count >= 100:
            break


def disasm_key_exports(pe):
    hr("9. DISASSEMBLY OF EXPORTED FUNCTION STUBS (top 10 exports)")
    if not HAS_CAPSTONE:
        print("  [Skipped - capstone not installed]")
        return
    if not pe.DIRECTORY_ENTRY_EXPORT:
        print("  No exports!")
        return

    named = [s for s in pe.DIRECTORY_ENTRY_EXPORT.symbols if s.name]
    named.sort(key=lambda s: s.name.decode('utf-8', errors='replace'))
    target_exports = named[:10]

    mode = CS_MODE_32 if pe.FILE_HEADER.Machine == 0x14c else CS_MODE_64
    md = Cs(CS_ARCH_X86, mode)
    md.detail = True
    img_base = pe.OPTIONAL_HEADER.ImageBase

    for sym in target_exports:
        name = sym.name.decode('utf-8', errors='replace')
        func_rva = sym.address
        print(f"\n  --- Export: {name} ---")
        print(f"  RVA: 0x{func_rva:08X}  VA: 0x{img_base + func_rva:08X}")

        func_data = None
        for sec in pe.sections:
            if sec.VirtualAddress <= func_rva < sec.VirtualAddress + sec.SizeOfRawData:
                offset = func_rva - sec.VirtualAddress
                raw = sec.get_data()
                func_data = raw[offset:offset + 256]
                break

        if not func_data:
            print("  Could not locate function bytes!")
            continue

        count = 0
        for insn in md.disasm(func_data, img_base + func_rva):
            print(f"    0x{insn.address:08X}:  {insn.mnemonic:8s} {insn.op_str}")
            count += 1
            if count >= 20:
                break
    print()


def dump_com_patterns(pe):
    hr("10. COM VTABLE PATTERNS (QueryInterface/AddRef/Release)")
    known_com_strings = [
        b"QueryInterface", b"AddRef", b"Release",
        b"IUnknown", b"IClassFactory", b"IDispatch",
        b"IPersistStream", b"IPersistStorage", b"IPersistFile",
        b"IStream", b"IStorage", b"ISequentialStream",
        b"IMoniker", b"IBindCtx", b"IEnumMoniker",
        b"IConnectionPointContainer", b"IConnectionPoint",
        b"IProvideClassInfo", b"ICallFactory", b"IManagedObject",
    ]

    found = defaultdict(list)
    for sec in pe.sections:
        if sec.SizeOfRawData == 0:
            continue
        data = sec.get_data()
        section_name = sec.Name.rstrip(b'\x00').decode('ascii', errors='replace')
        for cs in known_com_strings:
            offset = 0
            while True:
                pos = data.find(cs, offset)
                if pos == -1:
                    break
                rva = sec.VirtualAddress + pos
                found[cs.decode('ascii', errors='replace')].append((section_name, rva))
                offset = pos + 1

    if found:
        print("  Known COM interface strings found:")
        for name, locations in sorted(found.items()):
            print(f"    {name}: {len(locations)} occurrence(s)")
            for sec_name, rva in locations[:5]:
                print(f"      [{sec_name}+0x{rva:X}]")
            if len(locations) > 5:
                print(f"      ... and {len(locations)-5} more")
    else:
        print("  No well-known COM interface strings found in readable sections")

    print()
    print("  --- Searching for RTTI vtable layouts ---")
    vtable_patterns = defaultdict(int)
    for sec in pe.sections:
        if sec.SizeOfRawData == 0:
            continue
        data = sec.get_data()
        if not data or len(data) < 16:
            continue
        for marker in [b'?AV', b'?AU', b'?AA']:
            offset = 0
            while True:
                pos = data.find(marker, offset)
                if pos == -1:
                    break
                end = pos
                while end < len(data) and end < pos + 200 and data[end] not in (0, 0xFF):
                    end += 1
                try:
                    name = data[pos:end].decode('utf-8', errors='replace')
                    vtable_patterns[name] += 1
                except:
                    pass
                offset = pos + 1

    if vtable_patterns:
        print(f"  RTTI type names found: {len(vtable_patterns)} unique")
        for name, count in sorted(vtable_patterns.items()):
            try:
                print(f"    {name} (ref count: {count})")
            except UnicodeEncodeError:
                safe = name.encode('ascii', errors='replace').decode('ascii')
                print(f"    {safe} (ref count: {count})")


def dump_debug_info(pe):
    hr("11. DEBUG DIRECTORY / PDB INFO")
    if not hasattr(pe, 'DIRECTORY_ENTRY_DEBUG') or not pe.DIRECTORY_ENTRY_DEBUG:
        print("  No debug directory found!")
        return

    for dbg in pe.DIRECTORY_ENTRY_DEBUG:
        d = dbg.struct
        print(f"  Debug Entry:")
        print(f"    Characteristics:    0x{d.Characteristics:08X}")
        print(f"    TimeDateStamp:      0x{d.TimeDateStamp:08X}")
        print(f"    MajorVersion:       {d.MajorVersion}")
        print(f"    MinorVersion:       {d.MinorVersion}")
        print(f"    Type:               0x{d.Type:08X}", end="")
        dbg_types = {
            0: "UNKNOWN", 1: "COFF", 2: "CODEVIEW", 3: "FPO",
            4: "MISC", 5: "EXCEPTION", 6: "FIXUP",
            7: "OMAP_TO_SRC", 8: "OMAP_FROM_SRC", 9: "BORLAND",
            10: "RESERVED10", 11: "CLSID", 12: "VC_FEATURE",
            13: "POGO", 14: "ILTCG", 15: "MPX", 16: "REPRO",
        }
        print(f"  ({dbg_types.get(d.Type, 'Unknown')})")
        print(f"    SizeOfData:         0x{d.SizeOfData:08X}")
        print(f"    AddressOfRawData:   0x{d.AddressOfRawData:08X}")
        print(f"    PointerToRawData:   0x{d.PointerToRawData:08X}")

        if d.Type == 2 and d.AddressOfRawData:
            try:
                raw_data = pe.get_data(d.AddressOfRawData, d.SizeOfData)
                sig = struct.unpack_from('<I', raw_data, 0)[0]
                if sig == 0x53445352:  # 'RSDS'
                    guid = raw_data[4:20]
                    guid_parts = [
                        "{%02X%02X%02X%02X-" % (guid[3], guid[2], guid[1], guid[0]),
                        "%02X%02X-" % (guid[5], guid[4]),
                        "%02X%02X-" % (guid[7], guid[6]),
                        "%02X%02X-" % (guid[8], guid[9]),
                        "%02X%02X%02X%02X%02X%02X}" % (guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]),
                    ]
                    guid_str = "".join(guid_parts)
                    age = struct.unpack_from('<I', raw_data, 20)[0]
                    pdb_name = raw_data[24:].split(b'\x00')[0].decode('utf-8', errors='replace')
                    print(f"    --- CodeView PDB 7.0 ---")
                    print(f"    GUID:  {guid_str}")
                    print(f"    Age:   {age}")
                    print(f"    PDB:   {pdb_name}")
                elif (sig & 0xFFFFFFFF) == 0x3031424E:  # 'NB10'
                    print(f"    --- CodeView PDB 2.0 (NB10) ---")
                    print(f"    Raw: {raw_data[:64].hex()}")
            except Exception as e:
                print(f"    [Error reading CodeView: {e}]")


def dump_delay_imports(pe):
    hr("12. DELAY-LOADED IMPORTS")
    if not hasattr(pe, 'DIRECTORY_ENTRY_DELAY_IMPORT') or not pe.DIRECTORY_ENTRY_DELAY_IMPORT:
        print("  No delay-loaded imports found!")
        return

    for entry in pe.DIRECTORY_ENTRY_DELAY_IMPORT:
        dll_name = entry.dll.decode('utf-8', errors='replace') if entry.dll else "(unknown)"
        print(f"  --- {dll_name} ---")
        for func in entry.imports:
            if func.name:
                fname = func.name.decode('utf-8', errors='replace')
                print(f"    {fname}  (RVA: 0x{func.address:08X})")
            else:
                print(f"    ordinal #{func.ordinal}  (RVA: 0x{func.address:08X})")
        print()


def dump_tls(pe):
    hr("13. TLS CALLBACKS")
    tls_dir = pe.OPTIONAL_HEADER.DATA_DIRECTORY[9]
    if not tls_dir.VirtualAddress:
        print("  No TLS directory!")
        return

    print(f"  TLS Directory RVA: 0x{tls_dir.VirtualAddress:08X}")
    print(f"  TLS Directory Size: 0x{tls_dir.Size:08X}")

    if not hasattr(pe, 'DIRECTORY_ENTRY_TLS') or not pe.DIRECTORY_ENTRY_TLS:
        print("  TLS entries not parsed (may exist but not populated)")
        return

    tls = pe.DIRECTORY_ENTRY_TLS
    if tls.struct:
        s = tls.struct
        if pe.OPTIONAL_HEADER.Magic == 0x10b:
            print(f"    StartAddressOfRawData: 0x{s.StartAddressOfRawData:08X}")
            print(f"    EndAddressOfRawData:   0x{s.EndAddressOfRawData:08X}")
            print(f"    AddressOfIndex:        0x{s.AddressOfIndex:08X}")
            print(f"    AddressOfCallBacks:    0x{s.AddressOfCallBacks:08X}")
            print(f"    SizeOfZeroFill:        0x{s.SizeOfZeroFill:08X}")
            print(f"    Characteristics:       0x{s.Characteristics:08X}")
        else:
            print(f"    StartAddressOfRawData: 0x{s.StartAddressOfRawData:016X}")
            print(f"    EndAddressOfRawData:   0x{s.EndAddressOfRawData:016X}")
            print(f"    AddressOfIndex:        0x{s.AddressOfIndex:016X}")
            print(f"    AddressOfCallBacks:    0x{s.AddressOfCallBacks:016X}")
            print(f"    SizeOfZeroFill:        0x{s.SizeOfZeroFill:08X}")
            print(f"    Characteristics:       0x{s.Characteristics:08X}")

    if tls.callbacks:
        print(f"\n    TLS Callbacks: {len(tls.callbacks)}")
        for i, cb in enumerate(tls.callbacks):
            print(f"    [{i}] 0x{cb:016X}")
    else:
        print("    No TLS callbacks found")


def dump_load_config(pe):
    hr("14. LOAD CONFIG (CFG, SEH, HotPatch)")
    lc_dir = pe.OPTIONAL_HEADER.DATA_DIRECTORY[10]
    if not lc_dir.VirtualAddress:
        print("  No Load Config directory!")
        return

    print(f"  Load Config Directory RVA: 0x{lc_dir.VirtualAddress:08X}")
    print(f"  Load Config Directory Size: 0x{lc_dir.Size:08X}")

    if not hasattr(pe, 'DIRECTORY_ENTRY_LOAD_CONFIG') or not pe.DIRECTORY_ENTRY_LOAD_CONFIG:
        print("  Load config entries not parsed")
        return

    lc = pe.DIRECTORY_ENTRY_LOAD_CONFIG
    s = lc.struct
    print(f"  --- Load Config Entry ---")
    print(f"    Size:                          0x{s.Size:08X}")
    print(f"    TimeDateStamp:                 0x{s.TimeDateStamp:08X}")
    print(f"    MajorVersion:                  {s.MajorVersion}")
    print(f"    MinorVersion:                  {s.MinorVersion}")
    print(f"    GlobalFlagsClear:              0x{s.GlobalFlagsClear:08X}")
    print(f"    GlobalFlagsSet:                0x{s.GlobalFlagsSet:08X}")
    print(f"    CriticalSectionDefaultTimeout: 0x{s.CriticalSectionDefaultTimeout:08X}")
    if pe.OPTIONAL_HEADER.Magic == 0x10b:
        print(f"    DeCommitFreeBlockThreshold:    0x{s.DeCommitFreeBlockThreshold:08X}")
        print(f"    DeCommitTotalFreeThreshold:    0x{s.DeCommitTotalFreeThreshold:08X}")
        print(f"    LockPrefixTable:               0x{s.LockPrefixTable:08X}")
        print(f"    MaximumAllocationSize:         0x{s.MaximumAllocationSize:08X}")
        print(f"    VirtualMemoryThreshold:        0x{s.VirtualMemoryThreshold:08X}")
        print(f"    ProcessAffinityMask:           0x{s.ProcessAffinityMask:08X}")
    else:
        print(f"    DeCommitFreeBlockThreshold:    0x{s.DeCommitFreeBlockThreshold:016X}")
        print(f"    DeCommitTotalFreeThreshold:    0x{s.DeCommitTotalFreeThreshold:016X}")
        print(f"    LockPrefixTable:               0x{s.LockPrefixTable:016X}")
        print(f"    MaximumAllocationSize:         0x{s.MaximumAllocationSize:016X}")
        print(f"    VirtualMemoryThreshold:        0x{s.VirtualMemoryThreshold:016X}")
        print(f"    ProcessAffinityMask:           0x{s.ProcessAffinityMask:016X}")
    print(f"    ProcessHeapFlags:              0x{s.ProcessHeapFlags:08X}")
    print(f"    CSDVersion:                    {s.CSDVersion}")
    if hasattr(s, 'DependentLoadFlags'):
        print(f"    DependentLoadFlags:            0x{s.DependentLoadFlags:08X}")
    else:
        print(f"    DependentLoadFlags:            (not present in this LoadConfig size)")
    print(f"    EditList:                      0x{s.EditList:016X}")
    print(f"    SecurityCookie:                0x{s.SecurityCookie:016X}")
    if hasattr(s, 'SEHandlerTable'):
        print(f"    SEHandlerTable:                0x{s.SEHandlerTable:016X}")
    if hasattr(s, 'SEHandlerCount'):
        print(f"    SEHandlerCount:                {s.SEHandlerCount}")
    if hasattr(s, 'GuardCFCheckFunctionPointer'):
        print(f"    GuardCFCheckFunctionPointer:   0x{s.GuardCFCheckFunctionPointer:016X}")
    if hasattr(s, 'GuardCFDispatchFunctionPointer'):
        print(f"    GuardCFDispatchFunctionPointer:0x{s.GuardCFDispatchFunctionPointer:016X}")
    if hasattr(s, 'GuardCFFunctionTable'):
        print(f"    GuardCFFunctionTable:          0x{s.GuardCFFunctionTable:016X}")
    if hasattr(s, 'GuardCFFunctionCount'):
        print(f"    GuardCFFunctionCount:          {s.GuardCFFunctionCount}")
    if hasattr(s, 'GuardFlags'):
        print(f"    GuardFlags:                    0x{s.GuardFlags:08X}")
        gf = s.GuardFlags
        if gf & 0x0100:
            print(f"      -> IMAGE_GUARD_CF_INSTRUMENTED")
        if gf & 0x0200:
            print(f"      -> IMAGE_GUARD_CFW_INSTRUMENTED")
        if gf & 0x0400:
            print(f"      -> IMAGE_GUARD_CF_FUNCTION_TABLE_PRESENT")
        if gf & 0x0800:
            print(f"      -> IMAGE_GUARD_SECURITY_COOKIE")
        if gf & 0x1000:
            print(f"      -> IMAGE_GUARD_CF_FUNCTION_TABLE setAddress_based")
        if gf & 0x2000:
            print(f"      -> IMAGE_GUARD_RF_INSTRUMENTED")
        if gf & 0x4000:
            print(f"      -> IMAGE_GUARD_RF_ENABLE")
        if gf & 0x8000:
            print(f"      -> IMAGE_GUARD_RF_STRICT")
        if gf & 0x10000:
            print(f"      -> IMAGE_GUARD_CF_EXPORT_TABLE_PRESENT")
        if gf & 0x20000:
            print(f"      -> IMAGE_GUARD_CF_EXPORT_FUNCTION_POINTER_PRESENT")
        if gf & 0x40000:
            print(f"      -> IMAGE_GUARD_CF_FUNCTION_TABLE_SIZE_INFORMATION")

    if hasattr(s, 'SEHandlerTable') and hasattr(s, 'SEHandlerCount') and s.SEHandlerTable:
        print(f"\n    SE Handler Table entries:")
        try:
            data = pe.get_data(s.SEHandlerTable, s.SEHandlerCount * 4)
            for i in range(min(s.SEHandlerCount, 50)):
                handler_rva = struct.unpack_from('<I', data, i * 4)[0]
                print(f"      [{i}] RVA: 0x{handler_rva:08X}")
            if s.SEHandlerCount > 50:
                print(f"      ... and {s.SEHandlerCount - 50} more handlers")
        except:
            print("      (could not read SE handler table)")

    if hasattr(s, 'GuardCFFunctionTable') and hasattr(s, 'GuardCFFunctionCount') and s.GuardCFFunctionTable and s.GuardCFFunctionCount:
        print(f"\n    CFG Guard Function Table entries: {s.GuardCFFunctionCount}")
        try:
            data = pe.get_data(s.GuardCFFunctionTable, s.GuardCFFunctionCount * 4 if pe.OPTIONAL_HEADER.Magic == 0x10b else s.GuardCFFunctionCount * 8)
            count_to_show = min(s.GuardCFFunctionCount, 50)
            entry_size = 4 if pe.OPTIONAL_HEADER.Magic == 0x10b else 8
            for i in range(count_to_show):
                func_rva = struct.unpack_from('<I', data, i * entry_size)[0]
                print(f"      [{i}] RVA: 0x{func_rva:08X}")
            if s.GuardCFFunctionCount > 50:
                print(f"      ... and {s.GuardCFFunctionCount - 50} more entries")
        except Exception as e:
            print(f"      (could not read CFG function table: {e})")


def dump_relocations(pe):
    hr("15. RELOCATIONS SUMMARY")
    if not hasattr(pe, 'DIRECTORY_ENTRY_BASERELOC') or not pe.DIRECTORY_ENTRY_BASERELOC:
        print("  No relocations found!")
        return

    print(f"  Total relocation blocks: {len(pe.DIRECTORY_ENTRY_BASERELOC)}")
    total_relocs = 0
    type_counts = defaultdict(int)
    type_names = {
        0: "ABSOLUTE", 1: "HIGH", 2: "LOW", 3: "HIGHLOW",
        4: "HIGHADJ", 5: "MIPS_JMPADDR", 7: "ARM_movp",
        8: "THUMB_MOV32", 9: "MIPS_JMPADDR16", 10: "DIR64",
        11: "HIGH32_NO_ADJ", 12: "MIPS_JMPADDR32", 13: "DIR64_NB",
    }

    for block in pe.DIRECTORY_ENTRY_BASERELOC:
        block_size = block.struct.SizeOfBlock
        num_entries = (block_size - 8) // 2
        total_relocs += num_entries

        if hasattr(block, 'relocations') and block.relocations:
            for r in block.relocations:
                rtype = (r.type >> 12) & 0xF
                type_counts[type_names.get(rtype, f"TYPE_{rtype}")] += 1

    print(f"  Total individual relocations: {total_relocs}")
    print()
    print("  Relocation Type Distribution:")
    for tname, count in sorted(type_counts.items()):
        print(f"    {tname:20s}: {count}")
    print()
    print("  First 50 relocation blocks:")
    for i, block in enumerate(pe.DIRECTORY_ENTRY_BASERELOC[:50]):
        print(f"    Block {i:3d}: RVA=0x{block.struct.VirtualAddress:08X}  "
              f"Size=0x{block.struct.SizeOfBlock:08X}  "
              f"Entries={(block.struct.SizeOfBlock-8)//2}")
    if len(pe.DIRECTORY_ENTRY_BASERELOC) > 50:
        print(f"    ... and {len(pe.DIRECTORY_ENTRY_BASERELOC) - 50} more blocks")


def main():
    print(SEP)
    print("  EXHAUSTIVE PE ANALYSIS: MovieMakerCore.dll")
    print("  Windows Live Essentials 2012 - Movie Maker Core Module")
    print(f"  File: {DLL_PATH}")
    print(f"  File Size: {os.path.getsize(DLL_PATH):,} bytes ({os.path.getsize(DLL_PATH)/1024/1024:.2f} MB)")
    print(SEP)

    print("\n  Loading PE file...")
    pe = pefile.PE(DLL_PATH, fast_load=False)
    print("  PE loaded successfully.\n")

    try:
        dump_pe_headers(pe)
    except Exception as e:
        print(f"  [ERROR in PE headers]: {e}")
        import traceback
        traceback.print_exc()

    try:
        dump_sections(pe)
    except Exception as e:
        print(f"  [ERROR in sections]: {e}")
        import traceback
        traceback.print_exc()

    try:
        dump_imports(pe)
    except Exception as e:
        print(f"  [ERROR in imports]: {e}")
        import traceback
        traceback.print_exc()

    try:
        dump_exports(pe)
    except Exception as e:
        print(f"  [ERROR in exports]: {e}")
        import traceback
        traceback.print_exc()

    try:
        dump_rtti_and_classes(pe)
    except Exception as e:
        print(f"  [ERROR in RTTI/classes]: {e}")
        import traceback
        traceback.print_exc()

    try:
        dump_strings(pe)
    except Exception as e:
        print(f"  [ERROR in strings]: {e}")
        import traceback
        traceback.print_exc()

    try:
        dump_resources(pe)
    except Exception as e:
        print(f"  [ERROR in resources]: {e}")
        import traceback
        traceback.print_exc()

    try:
        disasm_entry_point(pe)
    except Exception as e:
        print(f"  [ERROR in entry point disasm]: {e}")
        import traceback
        traceback.print_exc()

    try:
        disasm_key_exports(pe)
    except Exception as e:
        print(f"  [ERROR in export disasm]: {e}")
        import traceback
        traceback.print_exc()

    try:
        dump_com_patterns(pe)
    except Exception as e:
        print(f"  [ERROR in COM patterns]: {e}")
        import traceback
        traceback.print_exc()

    try:
        dump_debug_info(pe)
    except Exception as e:
        print(f"  [ERROR in debug info]: {e}")
        import traceback
        traceback.print_exc()

    try:
        dump_delay_imports(pe)
    except Exception as e:
        print(f"  [ERROR in delay imports]: {e}")
        import traceback
        traceback.print_exc()

    try:
        dump_tls(pe)
    except Exception as e:
        print(f"  [ERROR in TLS]: {e}")
        import traceback
        traceback.print_exc()

    try:
        dump_load_config(pe)
    except Exception as e:
        print(f"  [ERROR in load config]: {e}")
        import traceback
        traceback.print_exc()

    try:
        dump_relocations(pe)
    except Exception as e:
        print(f"  [ERROR in relocations]: {e}")
        import traceback
        traceback.print_exc()

    hr("ANALYSIS COMPLETE")
    print(f"  File analyzed: {DLL_PATH}")
    print(f"  Total size: {os.path.getsize(DLL_PATH):,} bytes")
    pe.close()


if __name__ == "__main__":
    main()
