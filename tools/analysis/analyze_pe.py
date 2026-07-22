#!/usr/bin/env python3
"""
PE Analysis Script for MovieMaker.exe and related binaries.
Performs: header parsing, imports/exports, strings, disassembly of entry point,
TLS, resources, relocations, version info.
"""

import pefile
import os
import sys
import struct
from collections import defaultdict

def analyze_pe(filepath):
    name = os.path.basename(filepath)
    size = os.path.getsize(filepath)
    print(f"\n{'='*80}")
    print(f"  {name}  ({size:,} bytes)")
    print(f"{'='*80}")

    pe = pefile.PE(filepath, fast_load=False)

    # --- Basic Header Info ---
    print(f"\n[HEADER]")
    print(f"  Machine:            0x{pe.FILE_HEADER.Machine:04X}")
    print(f"  NumberOfSections:   {pe.FILE_HEADER.NumberOfSections}")
    print(f"  TimeDateStamp:      0x{pe.FILE_HEADER.TimeDateStamp:08X}")
    print(f"  PointerToSymbolTable: 0x{pe.FILE_HEADER.PointerToSymbolTable:08X}")
    print(f"  NumberOfSymbols:    {pe.FILE_HEADER.NumberOfSymbols}")
    print(f"  Characteristics:    0x{pe.FILE_HEADER.Characteristics:04X}")

    chars = []
    if pe.FILE_HEADER.Characteristics & 0x0002: chars.append("EXECUTABLE_IMAGE")
    if pe.FILE_HEADER.Characteristics & 0x0020: chars.append("LARGE_ADDRESS_AWARE")
    if pe.FILE_HEADER.Characteristics & 0x0100: chars.append("32BIT_MACHINE")
    if pe.FILE_HEADER.Characteristics & 0x2000: chars.append("DLL")
    print(f"    Flags: {', '.join(chars) if chars else 'none'}")

    is_dll = bool(pe.FILE_HEADER.Characteristics & 0x2000)
    is_64 = pe.PE_TYPE == pefile.OPTIONAL_HEADER_MAGIC_PEPlus

    oh = pe.OPTIONAL_HEADER
    print(f"\n  PE Type:            {'PE32+' if is_64 else 'PE32'}")
    print(f"  ImageBase:          0x{oh.ImageBase:08X}")
    print(f"  EntryPoint RVA:     0x{oh.AddressOfEntryPoint:08X}")
    print(f"  SectionAlignment:   0x{oh.SectionAlignment:08X}")
    print(f"  FileAlignment:      0x{oh.FileAlignment:08X}")
    print(f"  SizeOfImage:        0x{oh.SizeOfImage:08X}")
    print(f"  SizeOfHeaders:      0x{oh.SizeOfHeaders:08X}")
    print(f"  Subsystem:          {oh.Subsystem} ({'GUI' if oh.Subsystem == 2 else 'CONSOLE' if oh.Subsystem == 3 else 'unknown'})")
    print(f"  DllCharacteristics: 0x{oh.DllCharacteristics:04X}")

    dll_chars = []
    if oh.DllCharacteristics & 0x0020: dll_chars.append("HIGH_ENTROPY_VA")
    if oh.DllCharacteristics & 0x0040: dll_chars.append("DYNAMIC_BASE")
    if oh.DllCharacteristics & 0x0080: dll_chars.append("FORCE_INTEGRITY")
    if oh.DllCharacteristics & 0x0100: dll_chars.append("NX_COMPAT")
    if oh.DllCharacteristics & 0x0400: dll_chars.append("NO_SEH")
    if oh.DllCharacteristics & 0x0800: dll_chars.append("NO_BIND")
    if oh.DllCharacteristics & 0x1000: dll_chars.append("APPCONTAINER")
    if oh.DllCharacteristics & 0x2000: dll_chars.append("WDM_DRIVER")
    if oh.DllCharacteristics & 0x4000: dll_chars.append("GUARD_CF")
    print(f"    DllChars: {', '.join(dll_chars) if dll_chars else 'none'}")

    # --- Sections ---
    print(f"\n[SECTIONS]")
    print(f"  {'Name':<10} {'VirtAddr':>10} {'VirtSize':>10} {'RawAddr':>10} {'RawSize':>10} {'Chars':>12}")
    print(f"  {'-'*10} {'-'*10} {'-'*10} {'-'*10} {'-'*10} {'-'*12}")
    for s in pe.sections:
        name_str = s.Name.decode('utf-8', errors='replace').rstrip('\x00')
        chars_str = f"0x{s.Characteristics:08X}"
        print(f"  {name_str:<10} 0x{s.VirtualAddress:08X} 0x{s.Misc_VirtualSize:08X} 0x{s.PointerToRawData:08X} 0x{s.SizeOfRawData:08X} {chars_str}")

    # --- Data Directories ---
    print(f"\n[DATA DIRECTORIES]")
    dir_names = [
        "EXPORT", "IMPORT", "RESOURCE", "EXCEPTION", "SECURITY",
        "BASERELOC", "DEBUG", "ARCHITECTURE", "GLOBALPTR", "TLS",
        "LOAD_CONFIG", "BOUND_IMPORT", "IAT", "DELAY_IMPORT",
        "COM_DESCRIPTOR", "RESERVED"
    ]
    for i, entry in enumerate(oh.DATA_DIRECTORY):
        if entry.VirtualAddress != 0 or entry.Size != 0:
            name_s = dir_names[i] if i < len(dir_names) else f"DIR_{i}"
            print(f"  [{name_s:>15}] VA=0x{entry.VirtualAddress:08X}  Size=0x{entry.Size:08X}")

    # --- Imports ---
    print(f"\n[IMPORTS]")
    if hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
        total_funcs = 0
        for entry in pe.DIRECTORY_ENTRY_IMPORT:
            dll_name = entry.dll.decode('utf-8', errors='replace')
            funcs = []
            for imp in entry.imports:
                if imp.name:
                    fname = imp.name.decode('utf-8', errors='replace')
                else:
                    fname = f"ordinal_{imp.ordinal}"
                funcs.append(fname)
            total_funcs += len(funcs)
            print(f"  {dll_name} ({len(funcs)} functions):")
            for f in funcs:
                print(f"    {f}")
        print(f"\n  Total imported functions: {total_funcs}")
    else:
        print("  No standard import table found.")

    # --- Exports ---
    if is_dll and hasattr(pe, 'DIRECTORY_ENTRY_EXPORT'):
        print(f"\n[EXPORTS]")
        for exp in pe.DIRECTORY_ENTRY_EXPORT.symbols:
            exp_name = exp.name.decode('utf-8', errors='replace') if exp.name else f"ordinal_{exp.ordinal}"
            print(f"  ordinal={exp.ordinal}  RVA=0x{exp.address:08X}  name={exp_name}")

    # --- Resources ---
    if hasattr(pe, 'DIRECTORY_ENTRY_RESOURCE'):
        print(f"\n[RESOURCES]")
        def show_resources(entries, depth=0):
            for entry in entries:
                try:
                    rt_str = pefile.RESOURCE_TYPE.get(entry.struct.Id, f"ID_{entry.struct.Id}")
                except:
                    rt_str = f"ID_{entry.struct.Id}"
                name = ""
                if entry.name:
                    try:
                        name = entry.name.decode('utf-8', errors='replace')
                    except:
                        name = str(entry.name)
                if hasattr(entry, 'data') and entry.data:
                    size = len(entry.data.struct.Data)
                else:
                    size = 0
                indent = "  " * (depth + 2)
                print(f"{indent}{rt_str}{' (' + name + ')' if name else ''} size={size}")
                if hasattr(entry, 'directory') and entry.directory:
                    show_resources(entry.directory, depth + 1)
        show_resources(pe.DIRECTORY_ENTRY_RESOURCE.directory)

    # --- TLS ---
    if hasattr(pe, 'DIRECTORY_ENTRY_TLS'):
        print(f"\n[TLS]")
        tls = pe.DIRECTORY_ENTRY_TLS.struct
        print(f"  StartAddressOfRawData: 0x{tls.StartAddressOfRawData:08X}")
        print(f"  EndAddressOfRawData:   0x{tls.EndAddressOfRawData:08X}")
        print(f"  AddressOfIndex:        0x{tls.AddressOfIndex:08X}")
        print(f"  AddressOfCallBacks:    0x{tls.AddressOfCallBacks:08X}")

    # --- Debug ---
    if hasattr(pe, 'DIRECTORY_ENTRY_DEBUG'):
        print(f"\n[DEBUG]")
        for dbg in pe.DIRECTORY_ENTRY_DEBUG:
            print(f"  Type={dbg.struct.Type}  TimeDateStamp=0x{dbg.struct.TimeDateStamp:08X}  SizeOfData={dbg.struct.SizeOfData}")
            if dbg.entry:
                try:
                    pdb = dbg.entry.PdbFileName.decode('utf-8', errors='replace')
                    print(f"  PDB: {pdb}")
                except:
                    pass

    # --- Version Info ---
    if hasattr(pe, 'VS_FIXEDFILEINFO'):
        vinfo = pe.VS_FIXEDFILEINFO[0]
        fv = vinfo.FileVersionMS, vinfo.FileVersionLS
        pv = vinfo.ProductVersionMS, vinfo.ProductVersionLS
        print(f"\n[VERSION INFO]")
        print(f"  FileVersion:    {(fv[0]>>16)&0xFFFF}.{fv[0]&0xFFFF}.{(fv[1]>>16)&0xFFFF}.{fv[1]&0xFFFF}")
        print(f"  ProductVersion: {(pv[0]>>16)&0xFFFF}.{pv[0]&0xFFFF}.{(pv[1]>>16)&0xFFFF}.{pv[1]&0xFFFF}")
        print(f"  FileOS:         0x{vinfo.FileOS:08X}")
        print(f"  FileType:       0x{vinfo.FileType:08X}")
        print(f"  Flags:          0x{vinfo.FileFlagsMask:08X}")

    # --- Delay Imports ---
    if hasattr(pe, 'DIRECTORY_ENTRY_DELAY_IMPORT'):
        print(f"\n[DELAY IMPORTS]")
        for entry in pe.DIRECTORY_ENTRY_DELAY_IMPORT:
            dll_name = entry.dll.decode('utf-8', errors='replace')
            funcs = []
            for imp in entry.imports:
                if imp.name:
                    fname = imp.name.decode('utf-8', errors='replace')
                else:
                    fname = f"ordinal_{imp.ordinal}"
                funcs.append(fname)
            print(f"  {dll_name} ({len(funcs)} functions):")
            for f in funcs:
                print(f"    {f}")

    # --- Relocations ---
    if hasattr(pe, 'DIRECTORY_ENTRY_BASERELOC'):
        print(f"\n[RELOCATIONS]")
        total_relocs = 0
        for base_reloc in pe.DIRECTORY_ENTRY_BASERELOC:
            for entry in base_reloc.entries:
                total_relocs += 1
        print(f"  Total relocation entries: {total_relocs}")

    pe.close()
    return filepath


def extract_strings(filepath, min_len=6):
    """Extract ASCII and UTF-16 strings from a PE file."""
    name = os.path.basename(filepath)
    print(f"\n{'='*80}")
    print(f"  STRING ANALYSIS: {name}")
    print(f"{'='*80}")

    with open(filepath, 'rb') as f:
        data = f.read()

    # ASCII strings
    ascii_strings = []
    current = []
    for byte in data:
        if 0x20 <= byte < 0x7F:
            current.append(chr(byte))
        else:
            if len(current) >= min_len:
                ascii_strings.append(''.join(current))
            current = []
    if len(current) >= min_len:
        ascii_strings.append(''.join(current))

    # UTF-16LE strings
    utf16_strings = []
    current = []
    for i in range(0, len(data) - 1, 2):
        code = struct.unpack_from('<H', data, i)[0]
        if 0x20 <= code < 0x7F:
            current.append(chr(code))
        else:
            if len(current) >= min_len:
                utf16_strings.append(''.join(current))
            current = []
    if len(current) >= min_len:
        utf16_strings.append(''.join(current))

    # Filter interesting strings
    interesting_patterns = [
        'http', 'https', 'ftp', 'www', '.dll', '.exe', '.xml', '.json',
        'Software\\', 'HKEY_', 'CLSID', 'Interface', 'Registry',
        'CreateFile', 'ReadFile', 'WriteFile', 'LoadLibrary', 'GetProcAddress',
        'CoCreateInstance', 'RegOpenKey', 'RegSetValue', 'SHGetFolderPath',
        'VirtualAlloc', 'VirtualFree', 'HeapAlloc', 'HeapFree',
        'memcpy', 'memset', 'malloc', 'free', 'new', 'delete',
        'MovieMaker', 'PhotoGallery', 'WindowsLive', 'WLX',
        'C:\\Program', '%s', '%d', '%ls', '%S',
        'ERROR', 'WARNING', 'FATAL', 'FAIL', 'SUCCESS',
        'http://', 'https://',
        'Microsoft', 'Windows', 'Essentials',
        'timeline', 'storyboard', 'transition', 'effect', 'title',
        'video', 'audio', 'track', 'clip', 'trim',
        'publish', 'export', 'render', 'encode', 'decode',
        'undo', 'redo', 'copy', 'paste', 'delete',
        'project', 'autoSave', 'save', 'load',
        'filter', 'effect', 'pan', 'zoom',
    ]

    print(f"\n  ASCII strings: {len(ascii_strings)} total")
    print(f"  UTF-16 strings: {len(utf16_strings)} total")

    print(f"\n  --- Notable ASCII Strings ---")
    notable_ascii = set()
    for s in ascii_strings:
        sl = s.lower()
        for pat in interesting_patterns:
            if pat.lower() in sl:
                notable_ascii.add(s)
                break
    for s in sorted(notable_ascii)[:200]:
        print(f"    {s[:120]}")

    print(f"\n  --- Notable UTF-16 Strings ---")
    notable_utf16 = set()
    for s in utf16_strings:
        sl = s.lower()
        for pat in interesting_patterns:
            if pat.lower() in sl:
                notable_utf16.add(s)
                break
    for s in sorted(notable_utf16)[:200]:
        print(f"    {s[:120]}")

    return ascii_strings, utf16_strings


if __name__ == '__main__':
    base = r"C:\Users\mcmco\Desktop\WMMR\undecomp"

    targets = [
        os.path.join(base, "Photo Gallery", "MovieMaker.exe"),
        os.path.join(base, "Photo Gallery", "MovieMakerCore.dll"),
        os.path.join(base, "Photo Gallery", "MovieMakerLang.dll"),
        os.path.join(base, "Photo Gallery", "MovieMakerPreviewClient.dll"),
        os.path.join(base, "Shared", "WLXMovieLibrary.dll"),
        os.path.join(base, "Shared", "uxcore.dll"),
        os.path.join(base, "Shared", "wldcore.dll"),
    ]

    for target in targets:
        if os.path.exists(target):
            try:
                analyze_pe(target)
                extract_strings(target)
            except Exception as e:
                print(f"\n  ERROR analyzing {target}: {e}")
        else:
            print(f"\n  NOT FOUND: {target}")
