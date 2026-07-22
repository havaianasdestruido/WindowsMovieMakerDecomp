#!/usr/bin/env python3
"""
Exhaustive PE Analysis Script for MovieMaker.exe
Windows Live Essentials 2012
"""

import pefile
import capstone
import struct
import sys
import os
from datetime import datetime

PE_PATH = r"C:\Users\mcmco\Desktop\WMMR\undecomp\Photo Gallery\MovieMaker.exe"

DIVIDER = "=" * 80
SUBDIVIDER = "-" * 80

def format_hex(val, width=8):
    if isinstance(val, int):
        return f"0x{val:0{width}X}"
    return str(val)

def dump_pe_header(pe):
    print(DIVIDER)
    print("1. FULL PE HEADER DUMP")
    print(DIVIDER)

    # DOS Header
    print("\n--- DOS HEADER ---")
    dos = pe.DOS_HEADER
    print(f"  e_magic:    {format_hex(dos.e_magic, 4)} (MZ)")
    print(f"  e_cblp:     {dos.e_cblp}")
    print(f"  e_cp:       {dos.e_cp}")
    print(f"  e_crlc:     {dos.e_crlc}")
    print(f"  e_cparhdr:  {dos.e_cparhdr}")
    print(f"  e_minalloc: {dos.e_minalloc}")
    print(f"  e_maxalloc: {dos.e_maxalloc}")
    print(f"  e_ss:       {format_hex(dos.e_ss, 4)}")
    print(f"  e_sp:       {format_hex(dos.e_sp, 4)}")
    print(f"  e_csum:     {format_hex(dos.e_csum, 4)}")
    print(f"  e_ip:       {format_hex(dos.e_ip, 4)}")
    print(f"  e_cs:       {format_hex(dos.e_cs, 4)}")
    print(f"  e_lfarlc:   {format_hex(dos.e_lfarlc, 4)}")
    print(f"  e_ovno:     {format_hex(dos.e_ovno, 4)}")
    print(f"  e_res:      {[format_hex(x, 4) for x in dos.e_res]}")
    print(f"  e_oemid:    {format_hex(dos.e_oemid, 4)}")
    print(f"  e_oeminfo:  {format_hex(dos.e_oeminfo, 4)}")
    print(f"  e_lfanew:   {format_hex(dos.e_lfanew, 8)} (PE header offset)")

    # COFF Header
    print("\n--- COFF (IMAGE_FILE_HEADER) ---")
    coff = pe.FILE_HEADER
    machine_names = {
        0x0: "IMAGE_FILE_MACHINE_UNKNOWN",
        0x14c: "IMAGE_FILE_MACHINE_I386",
        0x8664: "IMAGE_FILE_MACHINE_AMD64",
        0x1c0: "IMAGE_FILE_MACHINE_ARM",
        0xaa64: "IMAGE_FILE_MACHINE_ARM64",
    }
    print(f"  Machine:              {format_hex(coff.Machine, 4)} ({machine_names.get(coff.Machine, 'Unknown')})")
    print(f"  NumberOfSections:     {coff.NumberOfSections}")
    try:
        ts = datetime.fromtimestamp(coff.TimeDateStamp, tz=__import__('datetime').timezone.utc) if coff.TimeDateStamp else "N/A"
    except Exception:
        ts = "N/A"
    print(f"  TimeDateStamp:        {format_hex(coff.TimeDateStamp, 8)} ({ts})")
    print(f"  PointerToSymbolTable: {format_hex(coff.PointerToSymbolTable, 8)}")
    print(f"  NumberOfSymbols:      {coff.NumberOfSymbols}")
    print(f"  SizeOfOptionalHeader: {coff.SizeOfOptionalHeader}")
    print(f"  Characteristics:      {format_hex(coff.Characteristics, 4)}")
    chars = []
    flag_names = [
        (0x0001, "RELOCS_STRIPPED"),
        (0x0002, "EXECUTABLE_IMAGE"),
        (0x0004, "LINE_NUMS_STRIPPED"),
        (0x0008, "LOCAL_SYMS_STRIPPED"),
        (0x0010, "AGGRESSIVE_WS_TRIM"),
        (0x0020, "LARGE_ADDRESS_AWARE"),
        (0x0080, "BYTES_REVERSED_LO"),
        (0x0100, "32BIT_MACHINE"),
        (0x0200, "DEBUG_STRIPPED"),
        (0x0400, "REMOVABLE_RUN_FROM_SWAP"),
        (0x0800, "NET_RUN_FROM_SWAP"),
        (0x1000, "SYSTEM"),
        (0x2000, "DLL"),
        (0x4000, "UP_SYSTEM_ONLY"),
        (0x8000, "BYTES_REVERSED_HI"),
    ]
    for bit, name in flag_names:
        if coff.Characteristics & bit:
            chars.append(name)
    print(f"  Characteristics flags: {', '.join(chars)}")

    # Optional Header
    print("\n--- OPTIONAL HEADER ---")
    opt = pe.OPTIONAL_HEADER
    print(f"  Magic:                       {format_hex(opt.Magic, 4)} ({'PE32' if opt.Magic == 0x10b else 'PE32+' if opt.Magic == 0x20b else 'Unknown'})")
    print(f"  MajorLinkerVersion:          {opt.MajorLinkerVersion}")
    print(f"  MinorLinkerVersion:          {opt.MinorLinkerVersion}")
    print(f"  SizeOfCode:                  {format_hex(opt.SizeOfCode, 8)}")
    print(f"  SizeOfInitializedData:       {format_hex(opt.SizeOfInitializedData, 8)}")
    print(f"  SizeOfUninitializedData:     {format_hex(opt.SizeOfUninitializedData, 8)}")
    print(f"  AddressOfEntryPoint:         {format_hex(opt.AddressOfEntryPoint, 8)}")
    print(f"  BaseOfCode:                  {format_hex(opt.BaseOfCode, 8)}")
    if opt.Magic == 0x10b:
        print(f"  BaseOfData:                  {format_hex(opt.BaseOfData, 8)}")
        print(f"  ImageBase:                   {format_hex(opt.ImageBase, 8)}")
    else:
        print(f"  ImageBase:                   {format_hex(opt.ImageBase, 16)}")
    print(f"  SectionAlignment:            {format_hex(opt.SectionAlignment, 8)}")
    print(f"  FileAlignment:               {format_hex(opt.FileAlignment, 8)}")
    print(f"  MajorOperatingSystemVersion: {opt.MajorOperatingSystemVersion}")
    print(f"  MinorOperatingSystemVersion: {opt.MinorOperatingSystemVersion}")
    print(f"  MajorImageVersion:           {opt.MajorImageVersion}")
    print(f"  MinorImageVersion:           {opt.MinorImageVersion}")
    print(f"  MajorSubsystemVersion:       {opt.MajorSubsystemVersion}")
    print(f"  MinorSubsystemVersion:       {opt.MinorSubsystemVersion}")
    try:
        print(f"  Win32VersionValue:           {format_hex(opt.Win32VersionValue, 8)}")
    except AttributeError:
        pass
    print(f"  SizeOfImage:                 {format_hex(opt.SizeOfImage, 8)}")
    print(f"  SizeOfHeaders:               {format_hex(opt.SizeOfHeaders, 8)}")
    print(f"  CheckSum:                    {format_hex(opt.CheckSum, 8)}")
    subsystem_names = {
        0: "UNKNOWN",
        1: "NATIVE",
        2: "WINDOWS_GUI",
        3: "WINDOWS_CUI",
        5: "OS2_CUI",
        7: "POSIX_CUI",
        9: "WINDOWS_CE_GUI",
        10: "EFI_APPLICATION",
        11: "EFI_BOOT_SERVICE_DRIVER",
        12: "EFI_RUNTIME_DRIVER",
        13: "EFI_ROM",
        14: "XBOX",
        16: "WINDOWS_BOOT_APPLICATION",
    }
    print(f"  Subsystem:                   {opt.Subsystem} ({subsystem_names.get(opt.Subsystem, 'Unknown')})")
    print(f"  DllCharacteristics:          {format_hex(opt.DllCharacteristics, 4)}")
    dll_chars = []
    dll_flag_names = [
        (0x0020, "HIGH_ENTROPY_VA"),
        (0x0040, "DYNAMIC_BASE (ASLR)"),
        (0x0080, "FORCE_INTEGRITY_CHECKS"),
        (0x0100, "NX_COMPAT (DEP)"),
        (0x0200, "NO_ISOLATION"),
        (0x0400, "NO_SEH"),
        (0x0800, "NO_BIND"),
        (0x1000, "APPCONTAINER"),
        (0x2000, "WDM_DRIVER"),
        (0x4000, "GUARD_CF"),
        (0x8000, "TERMINAL_SERVER_AWARE"),
    ]
    for bit, name in dll_flag_names:
        if opt.DllCharacteristics & bit:
            dll_chars.append(name)
    print(f"  DllCharacteristics flags:    {', '.join(dll_chars)}")

    if opt.Magic == 0x10b:
        print(f"  SizeOfStackReserve:          {format_hex(opt.SizeOfStackReserve, 8)}")
        print(f"  SizeOfStackCommit:           {format_hex(opt.SizeOfStackCommit, 8)}")
        print(f"  SizeOfHeapReserve:           {format_hex(opt.SizeOfHeapReserve, 8)}")
        print(f"  SizeOfHeapCommit:            {format_hex(opt.SizeOfHeapCommit, 8)}")
    else:
        print(f"  SizeOfStackReserve:          {format_hex(opt.SizeOfStackReserve, 16)}")
        print(f"  SizeOfStackCommit:           {format_hex(opt.SizeOfStackCommit, 16)}")
        print(f"  SizeOfHeapReserve:           {format_hex(opt.SizeOfHeapReserve, 16)}")
        print(f"  SizeOfHeapCommit:            {format_hex(opt.SizeOfHeapCommit, 16)}")
    print(f"  LoaderFlags:                 {format_hex(opt.LoaderFlags, 8)}")
    print(f"  NumberOfRvaAndSizes:         {opt.NumberOfRvaAndSizes}")

    # Data Directories
    print("\n--- DATA DIRECTORIES ---")
    dir_names = [
        "EXPORT_TABLE",
        "IMPORT_TABLE",
        "RESOURCE_TABLE",
        "EXCEPTION_TABLE",
        "SECURITY_TABLE",
        "BASERELOC_TABLE",
        "DEBUG",
        "ARCHITECTURE",
        "GLOBALPTR",
        "TLS_TABLE",
        "LOAD_CONFIG_TABLE",
        "BOUND_IMPORT",
        "IAT",
        "DELAY_IMPORT_DESCRIPTOR",
        "CLR_RUNTIME_HEADER",
        "RESERVED",
    ]
    for i in range(min(opt.NumberOfRvaAndSizes, 16)):
        try:
            dd = opt.DATA_DIRECTORY[i]
            name = dir_names[i] if i < len(dir_names) else f"DIRECTORY_{i}"
            if dd.VirtualAddress != 0 or dd.Size != 0:
                print(f"  [{i:2d}] {name:30s} RVA: {format_hex(dd.VirtualAddress, 8)}  Size: {format_hex(dd.Size, 8)}")
            else:
                print(f"  [{i:2d}] {name:30s} (empty)")
        except Exception as e:
            print(f"  [{i:2d}] Error reading directory: {e}")

    # Security Cookie, SEH, CFG
    print("\n--- LOAD CONFIG DIRECTORY ---")
    try:
        if hasattr(pe, 'DIRECTORY_ENTRY_LOAD_CONFIG'):
            lc = pe.DIRECTORY_ENTRY_LOAD_CONFIG
            if hasattr(lc, 'struct'):
                s = lc.struct
                print(f"  Size:                     {format_hex(s.Size, 8)}")
                print(f"  TimeDateStamp:            {format_hex(s.TimeDateStamp, 8)}")
                print(f"  MajorVersion:             {s.MajorVersion}")
                print(f"  MinorVersion:             {s.MinorVersion}")
                print(f"  GlobalFlagsClear:         {format_hex(s.GlobalFlagsClear, 8)}")
                print(f"  GlobalFlagsSet:           {format_hex(s.GlobalFlagsSet, 8)}")
                print(f"  CriticalSectionDefaultTimeout: {s.CriticalSectionDefaultTimeout}")
                print(f"  DeCommitFreeBlockThreshold:    {format_hex(s.DeCommitFreeBlockThreshold, 8) if s.DeCommitFreeBlockThreshold else 'N/A'}")
                print(f"  DeCommitTotalFreeThreshold:    {format_hex(s.DeCommitTotalFreeThreshold, 8) if s.DeCommitTotalFreeThreshold else 'N/A'}")
                print(f"  LockPrefixTable:          {format_hex(s.LockPrefixTable, 8) if hasattr(s, 'LockPrefixTable') and s.LockPrefixTable else 'N/A'}")
                print(f"  MaximumAllocationSize:    {format_hex(s.MaximumAllocationSize, 8) if hasattr(s, 'MaximumAllocationSize') else 'N/A'}")
                print(f"  VirtualMemoryThreshold:   {format_hex(s.VirtualMemoryThreshold, 8) if hasattr(s, 'VirtualMemoryThreshold') else 'N/A'}")
                print(f"  ProcessAffinityMask:      {format_hex(s.ProcessAffinityMask, 8) if hasattr(s, 'ProcessAffinityMask') else 'N/A'}")
                print(f"  ProcessHeapFlags:         {format_hex(s.ProcessHeapFlags, 4) if hasattr(s, 'ProcessHeapFlags') else 'N/A'}")
                print(f"  CSDVersion:               {s.CSDVersion if hasattr(s, 'CSDVersion') else 'N/A'}")
                print(f"  DependentLoadFlags:       {format_hex(s.DependentLoadFlags, 4) if hasattr(s, 'DependentLoadFlags') and s.DependentLoadFlags else 'N/A'}")
                print(f"  EditList:                 {format_hex(s.EditList, 8) if hasattr(s, 'EditList') and s.EditList else 'N/A'}")
                print(f"  SecurityCookie:          {format_hex(s.SecurityCookie, 8) if hasattr(s, 'SecurityCookie') and s.SecurityCookie else 'N/A'}")
                print(f"  SEHandlerTable:          {format_hex(s.SEHandlerTable, 8) if hasattr(s, 'SEHandlerTable') and s.SEHandlerTable else 'N/A'}")
                print(f"  SEHandlerCount:          {s.SEHandlerCount if hasattr(s, 'SEHandlerCount') else 'N/A'}")
                if hasattr(s, 'GuardCFCheckFunctionPointer'):
                    print(f"  GuardCFCheckFunctionPointer:  {format_hex(s.GuardCFCheckFunctionPointer, 8)}")
                if hasattr(s, 'GuardCFDispatchFunctionPointer'):
                    print(f"  GuardCFDispatchFunctionPointer: {format_hex(s.GuardCFDispatchFunctionPointer, 8)}")
                if hasattr(s, 'GuardCFFunctionTable'):
                    print(f"  GuardCFFunctionTable:    {format_hex(s.GuardCFFunctionTable, 8)}")
                if hasattr(s, 'GuardCFFunctionCount'):
                    print(f"  GuardCFFunctionCount:    {format_hex(s.GuardCFFunctionCount, 8)}")
                if hasattr(s, 'GuardFlags'):
                    gf = s.GuardFlags
                    print(f"  GuardFlags:               {format_hex(gf, 8)}")
                    gflags = []
                    if gf & 0x00000100: gflags.append("CF_INSTRUMENTED")
                    if gf & 0x00000200: gflags.append("CF_FUNCTION_TABLE_PRESENT")
                    if gf & 0x00000400: gflags.append("SECURITY_COOKIE_UNUSED")
                    if gf & 0x00000800: gflags.append("PROTECT_DELAYLOAD_IAT")
                    if gf & 0x00001000: gflags.append("DELAYLOAD_IAT_IN_ITS OWN_SECTION")
                    if gf & 0x00002000: gflags.append("CF_EXPORT_SUPPRESSION_INFO_PRESENT")
                    if gf & 0x00004000: gflags.append("CF_ENABLE_EXPORT_SUPPRESSION")
                    if gf & 0x00008000: gflags.append("CF_LONGJMP_TABLE_PRESENT")
                    if gf & 0x01000000: gflags.append("EXCEPTIONS_HANDLER_TABLE")
                    print(f"  GuardFlags decoded:      {', '.join(gflags) if gflags else 'none'}")
            else:
                print("  Load config structure found but no struct attribute")
        else:
            print("  DIRECTORY_ENTRY_LOAD_CONFIG not present")
    except Exception as e:
        print(f"  Error reading load config: {e}")


def dump_sections(pe):
    print(DIVIDER)
    print("2. ALL SECTIONS")
    print(DIVIDER)
    print(f"\n{'Name':<10s} {'VirtAddr':>10s} {'VirtSize':>10s} {'RawAddr':>10s} {'RawSize':>10s} {'Relocs':>8s} {'Lines':>8s} {'Flags':>12s}  Entropy")
    print(SUBDIVIDER)
    for sec in pe.sections:
        name = sec.Name.decode('utf-8', errors='replace').rstrip('\x00')
        flags = format_hex(sec.Characteristics, 8)
        entropy = sec.get_entropy()
        print(f"{name:<10s} {format_hex(sec.VirtualAddress, 8):>10s} {format_hex(sec.Misc_VirtualSize, 8):>10s} "
              f"{format_hex(sec.PointerToRawData, 8):>10s} {format_hex(sec.SizeOfRawData, 8):>10s} "
              f"{sec.NumberOfRelocations:>8d} {sec.NumberOfLinenumbers:>8d} {flags:>12s}  {entropy:.4f}")
        flag_list = []
        sec_chars = sec.Characteristics
        if sec_chars & 0x00000020: flag_list.append("CODE")
        if sec_chars & 0x00000040: flag_list.append("INITIALIZED_DATA")
        if sec_chars & 0x00000080: flag_list.append("UNINITIALIZED_DATA")
        if sec_chars & 0x02000000: flag_list.append("EXECUTE")
        if sec_chars & 0x04000000: flag_list.append("READ")
        if sec_chars & 0x08000000: flag_list.append("WRITE")
        if sec_chars & 0x10000000: flag_list.append("SHARED")
        if sec_chars & 0x20000000: flag_list.append("EXECUTE_READ")
        if sec_chars & 0x40000000: flag_list.append("EXECUTE_WRITECOPY")
        if sec_chars & 0x80000000: flag_list.append("WRITECOPY")
        print(f"           Flags: {', '.join(flag_list)}")


def dump_imports(pe):
    print(DIVIDER)
    print("3. ALL IMPORTS")
    print(DIVIDER)
    if not hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
        print("  No import directory found.")
        return
    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        dll_name = entry.dll.decode('utf-8', errors='replace')
        print(f"\n  DLL: {dll_name}")
        for imp in entry.imports:
            if imp.name:
                func_name = imp.name.decode('utf-8', errors='replace')
                print(f"    Ordinal: ---  Name: {func_name}  Hint: {imp.hint}  RVA: {format_hex(imp.address)}")
            else:
                print(f"    Ordinal: {imp.ordinal:>5d}  Name: (by ordinal)  RVA: {format_hex(imp.address)}")


def dump_exports(pe):
    print(DIVIDER)
    print("4. ALL EXPORTS")
    print(DIVIDER)
    if not hasattr(pe, 'DIRECTORY_ENTRY_EXPORT'):
        print("  No export directory found.")
        return
    exp = pe.DIRECTORY_ENTRY_EXPORT
    print(f"  Export Directory RVA: {format_hex(exp.struct.VirtualAddress, 8)}")
    print(f"  Name: {exp.name.decode('utf-8', errors='replace') if exp.name else '(none)'}")
    print(f"  Ordinal Base: {exp.struct.OrdinalBase}")
    print(f"  NumberOfFunctions: {exp.struct.NumberOfFunctions}")
    print(f"  NumberOfNames: {exp.struct.NumberOfNames}")
    print(f"\n  {'Ordinal':>8s}  {'RVA':>10s}  {'Name':<60s}  Forwarder")
    print(f"  {'-'*8}  {'-'*10}  {'-'*60}  {'-'*30}")
    for exp_func in exp.symbols:
        name = exp_func.name.decode('utf-8', errors='replace') if exp_func.name else "(no name)"
        fwd = ""
        if exp_func.forwarder:
            fwd = exp_func.forwarder.decode('utf-8', errors='replace')
        print(f"  {exp_func.ordinal:>8d}  {format_hex(exp_func.address, 8):>10s}  {name:<60s}  {fwd}")


def extract_strings(data, min_length=4):
    """Extract ASCII and UTF-16LE strings from binary data."""
    ascii_strings = []
    utf16_strings = []

    # ASCII strings
    current = []
    for b in data:
        if 0x20 <= b < 0x7f:
            current.append(chr(b))
        else:
            if len(current) >= min_length:
                ascii_strings.append(''.join(current))
            current = []
    if len(current) >= min_length:
        ascii_strings.append(''.join(current))

    # UTF-16LE strings
    i = 0
    current = []
    while i < len(data) - 1:
        lo = data[i]
        hi = data[i + 1]
        code = lo | (hi << 8)
        if 0x20 <= code < 0x7f:
            current.append(chr(code))
        else:
            if len(current) >= min_length:
                utf16_strings.append(''.join(current))
            current = []
        i += 2
    if len(current) >= min_length:
        utf16_strings.append(''.join(current))

    return ascii_strings, utf16_strings


def dump_strings(pe):
    print(DIVIDER)
    print("5. STRINGS (min length 4)")
    print(DIVIDER)

    # .rsrc section strings
    print("\n--- Strings from .rsrc section ---")
    for sec in pe.sections:
        name = sec.Name.decode('utf-8', errors='replace').rstrip('\x00')
        if '.rsrc' in name:
            data = sec.get_data()
            ascii_s, utf16_s = extract_strings(data)
            print(f"\n  .rsrc ASCII strings ({len(ascii_s)} found):")
            for s in ascii_s[:500]:
                print(f"    {s}")
            if len(ascii_s) > 500:
                print(f"    ... and {len(ascii_s) - 500} more")
            print(f"\n  .rsrc UTF-16 strings ({len(utf16_s)} found):")
            for s in utf16_s[:500]:
                print(f"    {s}")
            if len(utf16_s) > 500:
                print(f"    ... and {len(utf16_s) - 500} more")

    # Code/data sections
    code_sections = ['.text', '.rdata', '.data', '.idata', '.edata', '.pdata']
    print("\n--- Strings from code/data sections ---")
    for sec in pe.sections:
        name = sec.Name.decode('utf-8', errors='replace').rstrip('\x00')
        if name in code_sections or (not name.startswith('.rsrc')):
            data = sec.get_data()
            ascii_s, utf16_s = extract_strings(data)
            if ascii_s or utf16_s:
                print(f"\n  Section '{name}' ASCII strings ({len(ascii_s)} found):")
                for s in ascii_s[:200]:
                    print(f"    {s}")
                if len(ascii_s) > 200:
                    print(f"    ... and {len(ascii_s) - 200} more")
                print(f"  Section '{name}' UTF-16 strings ({len(utf16_s)} found):")
                for s in utf16_s[:200]:
                    print(f"    {s}")
                if len(utf16_s) > 200:
                    print(f"    ... and {len(utf16_s) - 200} more")


def dump_version_info(pe):
    print(DIVIDER)
    print("6. VERSION INFORMATION RESOURCES")
    print(DIVIDER)
    if not hasattr(pe, 'DIRECTORY_ENTRY_RESOURCE'):
        print("  No resource directory found.")
        return

    # Search all resource entries for VS_VERSION_INFO by looking for the magic string
    # in the resource data
    found_version = False
    if hasattr(pe, 'DIRECTORY_ENTRY_RESOURCE'):
        for directory in pe.DIRECTORY_ENTRY_RESOURCE.entries:
            if not (hasattr(directory, 'directory') and directory.directory):
                continue
            for sub_entry in directory.directory.entries:
                if not (hasattr(sub_entry, 'directory') and sub_entry.directory):
                    continue
                for lang_entry in sub_entry.directory.entries:
                    if not (hasattr(lang_entry, 'data') and lang_entry.data):
                        continue
                    data_rva = lang_entry.data.struct.OffsetToData
                    size = lang_entry.data.struct.Size
                    data = pe.get_memory_mapped_image()[data_rva:data_rva + size]
                    if len(data) < 6:
                        continue
                    # Check if this looks like a VS_VERSION_INFO resource
                    try:
                        text = data.decode('utf-16-le', errors='replace')
                        if 'VS_VERSION_INFO' in text:
                            found_version = True
                            print(f"\n  VS_VERSION_INFO resource (size: {size} bytes, RVA: {format_hex(data_rva, 8)}):")
                            wLength = struct.unpack_from('<H', data, 0)[0]
                            wValueLength = struct.unpack_from('<H', data, 2)[0]
                            wType = struct.unpack_from('<H', data, 4)[0]
                            print(f"    wLength:      {wLength}")
                            print(f"    wValueLength: {wValueLength}")
                            print(f"    wType:        {wType}")
                            offset = 6
                            key_data = data[offset:offset + 64]
                            key = key_data.decode('utf-16-le', errors='replace').split('\x00')[0]
                            print(f"    Key:          '{key}'")
                            offset += (len(key) + 1) * 2
                            offset = (offset + 3) & ~3
                            if wValueLength >= 52 and len(data) >= offset + 52:
                                sig = struct.unpack_from('<I', data, offset)[0]
                                struc_ver = struct.unpack_from('<I', data, offset + 4)[0]
                                file_ver_ms = struct.unpack_from('<I', data, offset + 8)[0]
                                file_ver_ls = struct.unpack_from('<I', data, offset + 12)[0]
                                prod_ver_ms = struct.unpack_from('<I', data, offset + 16)[0]
                                prod_ver_ls = struct.unpack_from('<I', data, offset + 20)[0]
                                file_flags_mask = struct.unpack_from('<I', data, offset + 24)[0]
                                file_flags = struct.unpack_from('<I', data, offset + 28)[0]
                                file_os = struct.unpack_from('<I', data, offset + 32)[0]
                                file_type = struct.unpack_from('<I', data, offset + 36)[0]
                                file_subtype = struct.unpack_from('<I', data, offset + 40)[0]
                                file_date_ms = struct.unpack_from('<I', data, offset + 44)[0]
                                file_date_ls = struct.unpack_from('<I', data, offset + 48)[0]
                                fv_hi = file_ver_ms >> 16
                                fv_lo = file_ver_ms & 0xFFFF
                                fv_bhi = file_ver_ls >> 16
                                fv_blo = file_ver_ls & 0xFFFF
                                pv_hi = prod_ver_ms >> 16
                                pv_lo = prod_ver_ms & 0xFFFF
                                pv_bhi = prod_ver_ls >> 16
                                pv_blo = prod_ver_ls & 0xFFFF
                                print(f"    Signature:    {format_hex(sig, 8)} ({'VS_FFI_SIGNATURE' if sig == 0xFEEF04BD else 'unknown'})")
                                print(f"    StrucVersion: {struc_ver >> 16}.{struc_ver & 0xFFFF}")
                                print(f"    FileVersion:  {fv_hi}.{fv_lo}.{fv_bhi}.{fv_blo}")
                                print(f"    ProdVersion:  {pv_hi}.{pv_lo}.{pv_bhi}.{pv_blo}")
                                print(f"    FileFlagsMask:{format_hex(file_flags_mask, 8)}")
                                print(f"    FileFlags:    {format_hex(file_flags, 8)}")
                                os_names = {
                                    0x00000001: "VOS_DOS_WINDOWS16",
                                    0x00000002: "VOS_OS216_PM16",
                                    0x00000003: "VOS_OS232_PM32",
                                    0x00000004: "VOS_NT_WINDOWS32",
                                    0x00000005: "VOS_DOS_WINDOWS32",
                                    0x00000010: "VOS_NT",
                                }
                                print(f"    FileOS:       {format_hex(file_os, 8)} ({os_names.get(file_os, 'Unknown')})")
                                ft_names = {0x1: "VFT_APP", 0x2: "VFT_DLL", 0x3: "VFT_DRV", 0x4: "VFT_FONT", 0x7: "VFT_STATIC_LIB"}
                                print(f"    FileType:     {format_hex(file_type, 8)} ({ft_names.get(file_type, 'Unknown')})")
                                print(f"    FileSubtype:  {format_hex(file_subtype, 8)}")
                                print(f"    FileDate:     {format_hex(file_date_ms, 8)}.{format_hex(file_date_ls, 8)}")
                            # Extract StringFileInfo
                            print("\n  --- StringFileInfo ---")
                            tags = ['CompanyName', 'FileDescription', 'FileVersion', 'InternalName',
                                    'LegalCopyright', 'LegalTrademarks', 'OriginalFilename',
                                    'ProductName', 'ProductVersion', 'Comments', 'AssemblyVersion']
                            for tag in tags:
                                idx = text.find(tag)
                                if idx >= 0:
                                    after = text[idx + len(tag):]
                                    null_idx = after.find('\x00')
                                    if null_idx >= 0:
                                        value_start = null_idx + 1
                                        value_end = after.find('\x00', value_start)
                                        if value_end < 0:
                                            value_end = value_start + 100
                                        value = after[value_start:value_end].strip()
                                        if value:
                                            print(f"    {tag}: {value}")
                    except Exception as e:
                        print(f"    Error parsing: {e}")
    if not found_version:
        print("  No VS_VERSION_INFO resource found via search.")


def dump_resources(pe):
    print(DIVIDER)
    print("7. RESOURCE TREE DUMP")
    print(DIVIDER)
    if not hasattr(pe, 'DIRECTORY_ENTRY_RESOURCE'):
        print("  No resource directory found.")
        return

    resource_type_names = {
        1: "RT_CURSOR",
        2: "RT_BITMAP",
        3: "RT_ICON",
        4: "RT_MENU",
        5: "RT_DIALOG",
        6: "RT_STRING",
        7: "RT_FONTDIR",
        8: "RT_FONT",
        9: "RT_ACCELERATOR",
        10: "RT_RCDATA",
        11: "RT_MESSAGETABLE",
        12: "RT_GROUP_CURSOR",
        14: "RT_GROUP_ICON",
        16: "RT_VERSION",
        17: "RT_DLGINCLUDE",
        19: "RT_PLUGPLAY",
        20: "RT_VXD",
        21: "RT_ANICURSOR",
        22: "RT_ANIICON",
        23: "RT_HTML",
        24: "RT_MANIFEST",
    }

    def print_resource_dir(directory, indent=0):
        prefix = "  " * indent
        for entry in directory.entries:
            if entry.name:
                try:
                    name_str = entry.name.name.decode('utf-16-le', errors='replace')
                except:
                    name_str = f"ID_{entry.name.struct.Id}"
            elif hasattr(entry.id, 'struct'):
                type_id = entry.id.struct.Id
                name_str = resource_type_names.get(type_id, f"Unknown({type_id})")
            else:
                name_str = "Unknown"

            if hasattr(entry, 'directory') and entry.directory:
                print(f"{prefix}Type: {name_str}")
                for sub_entry in entry.directory.entries:
                    if sub_entry.name:
                        try:
                            sub_name = sub_entry.name.name.decode('utf-16-le', errors='replace')
                        except:
                            sub_name = f"ID_{sub_entry.name.struct.Id}"
                    elif hasattr(sub_entry.id, 'struct'):
                        sub_name = str(sub_entry.id.struct.Id)
                    else:
                        sub_name = "Unknown"

                    if hasattr(sub_entry, 'directory') and sub_entry.directory:
                        print(f"{prefix}  Name: {sub_name}")
                        for lang_entry in sub_entry.directory.entries:
                            if lang_entry.data:
                                size = lang_entry.data.struct.Size
                                lang_id = lang_entry.id.struct.Id if hasattr(lang_entry.id, 'struct') else 0
                                codepage = 0
                                if hasattr(lang_entry.data, 'struct') and hasattr(lang_entry.data.struct, 'CodePage'):
                                    codepage = lang_entry.data.struct.CodePage
                                print(f"{prefix}    Lang: {lang_id}  CodePage: {codepage}  Size: {size} bytes  RVA: {format_hex(lang_entry.data.struct.OffsetToData, 8)}")
                            elif hasattr(sub_entry, 'directory') and sub_entry.directory:
                                print_resource_dir(sub_entry.directory, indent + 3)
                    else:
                        print(f"{prefix}  Name: {sub_name}")
            else:
                print(f"{prefix}Type: {name_str} (leaf)")

    for entry in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        if entry.name:
            try:
                name_str = entry.name.name.decode('utf-16-le', errors='replace')
            except:
                name_str = f"ID_{entry.name.struct.Id}"
        elif hasattr(entry.id, 'struct'):
            type_id = entry.id.struct.Id
            name_str = resource_type_names.get(type_id, f"Unknown({type_id})")
        else:
            name_str = "Unknown"

        print(f"\n{name_str}")
        if hasattr(entry, 'directory') and entry.directory:
            for sub_entry in entry.directory.entries:
                if sub_entry.name:
                    try:
                        sub_name = sub_entry.name.name.decode('utf-16-le', errors='replace')
                    except:
                        sub_name = f"ID_{sub_entry.name.struct.Id}"
                elif hasattr(sub_entry.id, 'struct'):
                    sub_name = str(sub_entry.id.struct.Id)
                else:
                    sub_name = "Unknown"
                print(f"  Name: {sub_name}")
                if hasattr(sub_entry, 'directory') and sub_entry.directory:
                    for lang_entry in sub_entry.directory.entries:
                        if lang_entry.data:
                            size = lang_entry.data.struct.Size
                            lang_id = lang_entry.id.struct.Id if hasattr(lang_entry.id, 'struct') else 0
                            print(f"    Lang: {lang_id}  Size: {size} bytes  RVA: {format_hex(lang_entry.data.struct.OffsetToData, 8)}")


def disassemble_entry_point(pe):
    print(DIVIDER)
    print("8. DISASSEMBLY OF ENTRY POINT (first 200 instructions)")
    print(DIVIDER)

    ep_rva = pe.OPTIONAL_HEADER.AddressOfEntryPoint
    print(f"  Entry Point RVA: {format_hex(ep_rva, 8)}")
    print(f"  Entry Point VA:  {format_hex(pe.OPTIONAL_HEADER.ImageBase + ep_rva, 8)}")

    # Find which section contains the entry point
    ep_section = None
    for sec in pe.sections:
        if sec.VirtualAddress <= ep_rva < sec.VirtualAddress + sec.Misc_VirtualSize:
            ep_section = sec
            break

    if ep_section is None:
        print("  ERROR: Entry point not found in any section!")
        return

    name = ep_section.Name.decode('utf-8', errors='replace').rstrip('\x00')
    print(f"  Entry Point Section: {name}")

    # Get code bytes from entry point
    ep_offset = ep_rva - ep_section.VirtualAddress
    code_data = ep_section.get_data()[ep_offset:ep_offset + 4096]

    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True

    count = 0
    print(f"\n  {'Address':>12s}  {'Bytes':<30s}  Instruction")
    print(f"  {'-'*12}  {'-'*30}  {'-'*40}")
    for insn in md.disasm(code_data, pe.OPTIONAL_HEADER.ImageBase + ep_rva):
        hex_bytes = ' '.join(f'{b:02x}' for b in insn.bytes)
        print(f"  {format_hex(insn.address, 8):>12s}  {hex_bytes:<30s}  {insn.mnemonic} {insn.op_str}")
        count += 1
        if count >= 200:
            break


def disassemble_iat(pe):
    print(DIVIDER)
    print("9. DISASSEMBLY OF IMPORTED THUNKS (IAT)")
    print(DIVIDER)
    if not hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
        print("  No import directory found.")
        return

    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True

    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        dll_name = entry.dll.decode('utf-8', errors='replace')
        print(f"\n  --- {dll_name} ---")
        for imp in entry.imports:
            if imp.address:
                func_name = imp.name.decode('utf-8', errors='replace') if imp.name else f"Ordinal_{imp.ordinal}"
                # Read the IAT entry (4 bytes for 32-bit thunk)
                try:
                    thunk_rva = imp.address - pe.OPTIONAL_HEADER.ImageBase
                    thunk_data = pe.get_memory_mapped_image()[thunk_rva:thunk_rva + 16]
                    hex_bytes = ' '.join(f'{b:02x}' for b in thunk_data[:8])
                    disasm = list(md.disasm(thunk_data, imp.address, count=1))
                    asm_str = ""
                    if disasm:
                        asm_str = f"{disasm[0].mnemonic} {disasm[0].op_str}"
                    print(f"    {func_name:<40s}  IAT @ {format_hex(imp.address, 8)}: {hex_bytes}  {asm_str}")
                except Exception as e:
                    print(f"    {func_name:<40s}  IAT @ {format_hex(imp.address, 8)}: Error: {e}")


def search_rtti(pe):
    print(DIVIDER)
    print("10. RTTI INFORMATION SEARCH")
    print(DIVIDER)

    # RTTI signatures and patterns
    # TypeDescriptor has RTTI Complete Object Locator signature at offset -4 (0x00000000 before the type)
    # We search for common RTTI string patterns and structure signatures

    sections_to_search = ['.rsrc', '.data', '.rdata', '.text']
    found_rtti = {}

    for sec in pe.sections:
        name = sec.Name.decode('utf-8', errors='replace').rstrip('\x00')
        if not any(s in name for s in sections_to_search):
            continue

        data = sec.get_data()
        data_va = sec.VirtualAddress + pe.OPTIONAL_HEADER.ImageBase

        # Search for TypeDescriptor patterns (RTTI type name strings preceded by ??_R0)
        # Type names often look like: ??_R0?AVClassName@@ or similar
        ascii_s, _ = extract_strings(data, min_length=4)
        rtti_strings = [s for s in ascii_s if '??_R0' in s or '??_R1' in s or '??_R2' in s or '??_R3' in s or '??_R4' in s]
        rtti_strings += [s for s in ascii_s if 'TypeDescriptor' in s or 'ClassHierarchy' in s or 'CompleteObject' in s or 'BaseClass' in s]
        rtti_strings += [s for s in ascii_s if '??_C@' in s and ('AV' in s or 'ABV' in s)]

        # Also search for C++ RTTI patterns: "class Type", "struct Type" in UTF-16
        _, utf16_s = extract_strings(data, min_length=4)
        rtti_strings += [s for s in utf16_s if '??_R0' in s or '??_R1' in s or '??_R2' in s or '??_R3' in s or '??_R4' in s]
        rtti_strings += [s for s in utf16_s if 'class ' in s or 'struct ' in s]

        if rtti_strings:
            print(f"\n  Section '{name}' - RTTI-related strings ({len(rtti_strings)} found):")
            for s in rtti_strings[:300]:
                print(f"    {s}")
            if len(rtti_strings) > 300:
                print(f"    ... and {len(rtti_strings) - 300} more")

        # Search for binary RTTI signatures
        # CompleteObjectLocator has a specific pattern
        # Look for sequences of DWORDs that could be RTTI structures
        for offset in range(0, len(data) - 16, 4):
            val1 = struct.unpack_from('<I', data, offset)[0]
            # TypeDescriptor: RTTITypeDescriptorSignature = 0 (first field)
            # ClassHierarchyDescriptor signature = 0
            # Look for potential TypeDescriptor signature values
            if val1 == 0 and offset + 4 < len(data):
                val2 = struct.unpack_from('<I', data, offset + 4)[0]
                val3 = struct.unpack_from('<I', data, offset + 8)[0] if offset + 8 < len(data) else 0
                # Potential RTTI patterns
                if val2 == 0 and val3 > 0x10000 and val3 < 0x80000000:
                    # Could be RTTI - check if there's a type name string reference nearby
                    va = data_va + offset
                    if not name in found_rtti:
                        found_rtti[name] = []
                    found_rtti[name].append({
                        'offset': offset,
                        'va': va,
                        'pattern': f"0x{val1:08X} 0x{val2:08X} 0x{val3:08X}"
                    })

    if found_rtti:
        print(f"\n  Binary RTTI structure candidates:")
        for sec_name, items in found_rtti.items():
            print(f"\n  Section '{sec_name}':")
            for item in items[:50]:
                print(f"    VA: {format_hex(item['va'], 8)}  Pattern: {item['pattern']}")
            if len(items) > 50:
                print(f"    ... and {len(items) - 50} more candidates")

    # Search for GUIDs in RTTI context
    print("\n  Searching for RTTI GUID patterns...")
    for sec in pe.sections:
        name = sec.Name.decode('utf-8', errors='replace').rstrip('\x00')
        data = sec.get_data()
        # Search for GUID-like patterns (16 bytes with specific structure)
        for offset in range(0, len(data) - 16, 1):
            if data[8:10] == b'\x00\x00' or True:
                guid_data = data[offset:offset + 16]
                guid_str = ("{" + f"{guid_data[3]:02x}{guid_data[2]:02x}{guid_data[1]:02x}{guid_data[0]:02x}-" \
                          f"{guid_data[5]:02x}{guid_data[4]:02x}-" \
                          f"{guid_data[7]:02x}{guid_data[6]:02x}-" \
                          f"{guid_data[8]:02x}{guid_data[9]:02x}-" \
                          f"{guid_data[10]:02x}{guid_data[11]:02x}{guid_data[12]:02x}{guid_data[13]:02x}{guid_data[14]:02x}{guid_data[15]:02x}" + "}")


def search_com_guids(pe):
    print(DIVIDER)
    print("11. COM INTERFACE IDs (IID/CLSID) AND GUIDs")
    print(DIVIDER)

    all_strings_ascii = []
    all_strings_utf16 = []

    for sec in pe.sections:
        name = sec.Name.decode('utf-8', errors='replace').rstrip('\x00')
        data = sec.get_data()
        ascii_s, utf16_s = extract_strings(data, min_length=4)
        all_strings_ascii.extend([(s, name) for s in ascii_s])
        all_strings_utf16.extend([(s, name) for s in utf16_s])

    # Search for IID_ and CLSID_ patterns
    iid_strings = [(s, n) for s, n in all_strings_ascii if s.startswith('IID_') or s.startswith('CLSID_') or s.startswith('LIBID_') or s.startswith('DIID_')]
    iid_strings += [(s, n) for s, n in all_strings_utf16 if s.startswith('IID_') or s.startswith('CLSID_') or s.startswith('LIBID_') or s.startswith('DIID_')]

    if iid_strings:
        print("\n  IID/CLSID/LIBID named strings:")
        for s, n in iid_strings:
            print(f"    [{n}] {s}")

    # Search for GUID format strings: {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
    import re
    guid_pattern = re.compile(r'\{[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\}')

    print("\n  GUID format strings found:")
    seen_guids = set()
    for s, n in all_strings_ascii:
        matches = guid_pattern.findall(s)
        for m in matches:
            if m not in seen_guids:
                seen_guids.add(m)
                print(f"    [{n}] {m}")
    for s, n in all_strings_utf16:
        matches = guid_pattern.findall(s)
        for m in matches:
            if m not in seen_guids:
                seen_guids.add(m)
                print(f"    [{n}] {m}")

    print(f"\n  Total unique GUIDs found: {len(seen_guids)}")

    # Search for binary GUID patterns
    print("\n  Binary GUID patterns (potential COM IIDs in data):")
    guid_count = 0
    for sec in pe.sections:
        name = sec.Name.decode('utf-8', errors='replace').rstrip('\x00')
        data = sec.get_data()
        for offset in range(0, len(data) - 16, 1):
            b = data[offset:offset + 16]
            # Check if it looks like a GUID (not all zeros, not all same byte)
            if b == b'\x00' * 16:
                continue
            if len(set(b)) < 4:
                continue
            va = sec.VirtualAddress + offset + pe.OPTIONAL_HEADER.ImageBase
            guid_str = ("{" + f"{b[3]:02x}{b[2]:02x}{b[1]:02x}{b[0]:02x}-{b[5]:02x}{b[4]:02x}-{b[7]:02x}{b[6]:02x}-{b[8]:02x}{b[9]:02x}-{b[10]:02x}{b[11]:02x}{b[12]:02x}{b[13]:02x}{b[14]:02x}{b[15]:02x}" + "}")
            # Only print first 100
            if guid_count < 100:
                print(f"    [{name}] VA {format_hex(va, 8)}: {guid_str}")
            guid_count += 1
    if guid_count > 100:
        print(f"    ... and {guid_count - 100} more binary GUIDs")
    print(f"  Total binary GUID candidates: {guid_count}")


def dump_debug_dir(pe):
    print(DIVIDER)
    print("12. DEBUG DIRECTORY INFO")
    print(DIVIDER)
    if not hasattr(pe, 'DIRECTORY_ENTRY_DEBUG'):
        print("  No debug directory found.")
        return

    debug_type_names = {
        0: "IMAGE_DEBUG_TYPE_UNKNOWN",
        1: "IMAGE_DEBUG_TYPE_COFF",
        2: "IMAGE_DEBUG_TYPE_CODEVIEW",
        3: "IMAGE_DEBUG_TYPE_FPO",
        4: "IMAGE_DEBUG_TYPE_MISC",
        5: "IMAGE_DEBUG_TYPE_EXCEPTION",
        6: "IMAGE_DEBUG_TYPE_FIXUP",
        7: "IMAGE_DEBUG_TYPE_OMAP_TO_SRC",
        8: "IMAGE_DEBUG_TYPE_OMAP_FROM_SRC",
        9: "IMAGE_DEBUG_TYPE_BORLAND",
        10: "IMAGE_DEBUG_TYPE_RESERVED10",
        11: "IMAGE_DEBUG_TYPE_CLSID",
        12: "IMAGE_DEBUG_TYPE_VC_FEATURE",
        13: "IMAGE_DEBUG_TYPE_POGO",
        14: "IMAGE_DEBUG_TYPE_ILTCG",
        15: "IMAGE_DEBUG_TYPE_MPX",
        16: "IMAGE_DEBUG_TYPE_REPRO",
    }

    for dbg in pe.DIRECTORY_ENTRY_DEBUG:
        print(f"\n  Debug Directory Entry:")
        print(f"    Characteristics:    {format_hex(dbg.struct.Characteristics, 8)}")
        try:
            ts = datetime.fromtimestamp(dbg.struct.TimeDateStamp, tz=__import__('datetime').timezone.utc) if dbg.struct.TimeDateStamp else "N/A"
        except Exception:
            ts = "N/A"
        print(f"    TimeDateStamp:      {format_hex(dbg.struct.TimeDateStamp, 8)} ({ts})")
        print(f"    MajorVersion:       {dbg.struct.MajorVersion}")
        print(f"    MinorVersion:       {dbg.struct.MinorVersion}")
        print(f"    Type:               {dbg.struct.Type} ({debug_type_names.get(dbg.struct.Type, 'Unknown')})")
        print(f"    SizeOfData:         {format_hex(dbg.struct.SizeOfData, 8)}")
        print(f"    AddressOfRawData:   {format_hex(dbg.struct.AddressOfRawData, 8)}")
        print(f"    PointerToRawData:   {format_hex(dbg.struct.PointerToRawData, 8)}")

        if dbg.struct.Type == 2:  # IMAGE_DEBUG_TYPE_CODEVIEW
            try:
                raw_offset = dbg.struct.PointerToRawData
                cv_data = pe.get_data(raw_offset, dbg.struct.SizeOfData)
                if len(cv_data) >= 4:
                    sig = struct.unpack_from('<I', cv_data, 0)[0]
                    sig_str = cv_data[:4].decode('ascii', errors='replace')
                    print(f"    CodeView Signature: {format_hex(sig, 8)} ({sig_str})")
                    if sig_str == 'RSDS':
                        # PDB 7.0
                        pdb_guid = cv_data[4:20]
                        age = struct.unpack_from('<I', cv_data, 20)[0]
                        pdb_path = cv_data[24:].decode('utf-8', errors='replace').rstrip('\x00')
                        guid_str = ("{" + f"{pdb_guid[3]:02x}{pdb_guid[2]:02x}{pdb_guid[1]:02x}{pdb_guid[0]:02x}-" \
                                  f"{pdb_guid[5]:02x}{pdb_guid[4]:02x}-" \
                                  f"{pdb_guid[7]:02x}{pdb_guid[6]:02x}-" \
                                  f"{pdb_guid[8]:02x}{pdb_guid[9]:02x}-" \
                                  f"{pdb_guid[10]:02x}{pdb_guid[11]:02x}{pdb_guid[12]:02x}{pdb_guid[13]:02x}{pdb_guid[14]:02x}{pdb_guid[15]:02x}" + "}")
                        print(f"    PDB GUID:           {guid_str}")
                        print(f"    PDB Age:            {age}")
                        print(f"    PDB Path:           {pdb_path}")
                    elif sig_str == 'NB10':
                        # PDB 2.0
                        offset_pdb = struct.unpack_from('<I', cv_data, 4)[0]
                        timestamp = struct.unpack_from('<I', cv_data, 8)[0]
                        age = struct.unpack_from('<I', cv_data, 12)[0]
                        pdb_path = cv_data[16:].decode('utf-8', errors='replace').rstrip('\x00')
                        print(f"    PDB Offset:         {format_hex(offset_pdb, 8)}")
                        print(f"    PDB Timestamp:      {format_hex(timestamp, 8)}")
                        print(f"    PDB Age:            {age}")
                        print(f"    PDB Path:           {pdb_path}")
            except Exception as e:
                print(f"    Error reading CodeView data: {e}")


def dump_delay_imports(pe):
    print(DIVIDER)
    print("14. DELAY-LOADED IMPORTS")
    print(DIVIDER)
    if not hasattr(pe, 'DIRECTORY_ENTRY_DELAY_IMPORT'):
        print("  No delay-loaded imports found.")
        return

    for dimport in pe.DIRECTORY_ENTRY_DELAY_IMPORT:
        dll_name = dimport.dll.decode('utf-8', errors='replace') if dimport.dll else "(unknown)"
        print(f"\n  DLL: {dll_name}")
        attrs = getattr(dimport.struct, 'Attributes', None)
        name_rva = getattr(dimport.struct, 'Name', None)
        mod_handle = getattr(dimport.struct, 'ModuleHandleRVA', None)
        diat_rva = getattr(dimport.struct, 'DelayImportAddressTableRVA', None)
        dint_rva = getattr(dimport.struct, 'DelayImportNameTableRVA', None)
        if attrs is not None:
            print(f"    Attributes:    {format_hex(attrs, 8)}")
        if name_rva is not None:
            print(f"    Name:          RVA {format_hex(name_rva, 8)}")
        if mod_handle is not None:
            print(f"    ModuleHandle:  RVA {format_hex(mod_handle, 8)}")
        if diat_rva is not None:
            print(f"    DelayImportAddressTable: RVA {format_hex(diat_rva, 8)}")
        if dint_rva is not None:
            print(f"    DelayImportNameTable:    RVA {format_hex(dint_rva, 8)}")
        if hasattr(dimport, 'imports'):
            for imp in dimport.imports:
                if imp.name:
                    func_name = imp.name.decode('utf-8', errors='replace')
                    print(f"      Name: {func_name}  Hint: {imp.hint}  VA: {format_hex(imp.address)}")
                else:
                    print(f"      Ordinal: {imp.ordinal}  VA: {format_hex(imp.address)}")


def dump_tls(pe):
    print(DIVIDER)
    print("15. TLS CALLBACKS")
    print(DIVIDER)

    tls = None
    try:
        tls = pe.get_directory_entry(14)  # IMAGE_DIRECTORY_ENTRY_TLS
    except:
        pass

    if tls is None or not hasattr(pe, 'DIRECTORY_ENTRY_TLS'):
        print("  No TLS directory found.")
        return

    if hasattr(pe, 'DIRECTORY_ENTRY_TLS'):
        tls_dir = pe.DIRECTORY_ENTRY_TLS
        if hasattr(tls_dir, 'struct'):
            s = tls_dir.struct
            print(f"  StartAddressOfRawData: {format_hex(s.StartAddressOfRawData, 8)}")
            print(f"  EndAddressOfRawData:   {format_hex(s.EndAddressOfRawData, 8)}")
            print(f"  AddressOfIndex:        {format_hex(s.AddressOfIndex, 8)}")
            print(f"  AddressOfCallBacks:    {format_hex(s.AddressOfCallBacks, 8)}")
            print(f"  SizeOfZeroFill:        {s.SizeOfZeroFill}")
            print(f"  Characteristics:       {format_hex(s.Characteristics, 8)}")

        if hasattr(tls_dir, 'callbacks') and tls_dir.callbacks:
            print(f"\n  TLS Callbacks ({len(tls_dir.callbacks)} found):")
            for i, cb in enumerate(tls_dir.callbacks):
                print(f"    [{i}] VA: {format_hex(cb, 8)}")
        else:
            print("  No TLS callbacks found.")


def dump_relocations(pe):
    print(DIVIDER)
    print("16. RELOCATION ENTRIES SUMMARY")
    print(DIVIDER)
    if not hasattr(pe, 'DIRECTORY_ENTRY_BASERELOC'):
        print("  No relocation directory found.")
        return

    total_relocs = 0
    reloc_types = {}
    type_names = {
        0: "IMAGE_REL_BASED_ABSOLUTE (padding)",
        1: "IMAGE_REL_BASED_HIGH",
        2: "IMAGE_REL_BASED_LOW",
        3: "IMAGE_REL_BASED_HIGHLOW",
        4: "IMAGE_REL_BASED_HIGHADJ",
        5: "IMAGE_REL_BASED_DIR64",
        6: "IMAGE_REL_BASED_IA64_IMM64",
        7: "IMAGE_REL_BASED_MIPS_JMPADDR",
        8: "IMAGE_REL_BASED_ARM_MOV32",
        9: "IMAGE_REL_BASED_THUMB_MOV32",
        10: "IMAGE_REL_BASED_MIPS_JMPADDR16",
        11: "IMAGE_REL_BASED_IA64_IMM12",
        12: "IMAGE_REL_BASED_DIR64_NB",
    }

    print(f"  {'Block RVA':>12s}  {'Size':>8s}  {'Entries':>8s}")
    print(f"  {'-'*12}  {'-'*8}  {'-'*8}")

    for base_reloc in pe.DIRECTORY_ENTRY_BASERELOC:
        num_entries = (base_reloc.struct.SizeOfBlock - 8) // 2
        total_relocs += num_entries
        rva = base_reloc.struct.VirtualAddress

        for entry in base_reloc.entries:
            reloc_type = entry.type
            if reloc_type in reloc_types:
                reloc_types[reloc_type] += 1
            else:
                reloc_types[reloc_type] = 1

        print(f"  {format_hex(rva, 8):>12s}  {base_reloc.struct.SizeOfBlock:>8d}  {num_entries:>8d}")

    print(f"\n  Total relocation blocks: {len(pe.DIRECTORY_ENTRY_BASERELOC)}")
    print(f"  Total relocation entries: {total_relocs}")
    print(f"\n  Relocation types breakdown:")
    for rtype, count in sorted(reloc_types.items()):
        type_name = type_names.get(rtype, f"Unknown({rtype})")
        print(f"    Type {rtype} ({type_name}): {count}")


def main():
    print(DIVIDER)
    print("EXHAUSTIVE PE ANALYSIS: MovieMaker.exe")
    print(f"File: {PE_PATH}")
    print(f"File size: {os.path.getsize(PE_PATH)} bytes")
    print(f"Analysis time: {datetime.now().isoformat()}")
    print(DIVIDER)

    pe = pefile.PE(PE_PATH)

    dump_pe_header(pe)
    print("\n")
    dump_sections(pe)
    print("\n")
    dump_imports(pe)
    print("\n")
    dump_exports(pe)
    print("\n")
    dump_strings(pe)
    print("\n")
    dump_version_info(pe)
    print("\n")
    dump_resources(pe)
    print("\n")
    disassemble_entry_point(pe)
    print("\n")
    disassemble_iat(pe)
    print("\n")
    search_rtti(pe)
    print("\n")
    search_com_guids(pe)
    print("\n")
    dump_debug_dir(pe)
    print("\n")
    dump_delay_imports(pe)
    print("\n")
    dump_tls(pe)
    print("\n")
    dump_relocations(pe)

    pe.close()
    print(f"\n{DIVIDER}")
    print("ANALYSIS COMPLETE")
    print(DIVIDER)


if __name__ == "__main__":
    main()
