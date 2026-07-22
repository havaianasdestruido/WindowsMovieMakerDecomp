#!/usr/bin/env python3
"""
Reverse Engineering Analysis of MovieMaker Supporting DLLs
Windows Live Essentials 2012
Analyzes PE headers, exports, imports, sections, strings, version info, and infers purpose.
"""

import os
import sys
import struct
import re
from collections import defaultdict, OrderedDict

try:
    import pefile
except ImportError:
    sys.exit("ERROR: pefile not installed. Run: pip install pefile")
try:
    import capstone
except ImportError:
    sys.exit("ERROR: capstone not installed. Run: pip install capstone")

BASE_DIR = r"C:\Users\mcmco\Desktop\WMMR\undecomp"

DLL_LIST = [
    r"Photo Gallery\MovieMakerLang.dll",
    r"Photo Gallery\MovieMakerPreviewClient.dll",
    r"Shared\WLXMovieLibrary.dll",
    r"Photo Gallery\WLXVideoTrim.dll",
    r"Photo Gallery\WLXPipeline.dll",
    r"Photo Gallery\WLXPipetran.dll",
    r"Photo Gallery\WLXCodecHost.exe",
    r"Photo Gallery\WLXTranscode.exe",
    r"Photo Gallery\WLXMediaPublishSubscribe.dll",
    r"Shared\WLMFReadWrite.dll",
    r"Shared\WLMFDS.dll",
    r"Photo Gallery\WLXMP4Parser.dll",
    r"Photo Gallery\WLXPhotoBase.dll",
    r"Photo Gallery\WLXPhotoGallery.exe",
    r"Photo Gallery\WLXSlideshow.dll",
    r"Photo Gallery\WLXPhotoCinematic.dll",
    r"Photo Gallery\WLXFaceRecognition.dll",
]

MACHINE_MAP = {
    0x0: "Unknown",
    0x14c: "x86 (i386)",
    0x8664: "x86-64 (AMD64)",
    0x1c0: "ARM",
    0xAA64: "ARM64",
    0x1c4: "ARM Thumb-2",
    0x200: "IA64 (Itanium)",
}

SUBSYSTEM_MAP = {
    0: "Unknown",
    1: "Native",
    2: "Windows GUI",
    3: "Windows Console",
    5: "OS/2 Console",
    7: "POSIX Console",
    9: "Windows CE",
    10: "EFI Application",
    11: "EFI Boot Service Driver",
    12: "EFI Runtime Driver",
    13: "EFI ROM",
    14: "Xbox",
    16: "Windows Boot Application",
}

CHARACTERISTICS_MAP = {
    0x0001: "RELOCS_STRIPPED",
    0x0002: "EXECUTABLE_IMAGE",
    0x0004: "LINE_NUMS_STRIPPED",
    0x0008: "LOCAL_SYMS_STRIPPED",
    0x0020: "LARGE_ADDRESS_AWARE",
    0x0080: "BYTES_REVERSED_LO",
    0x0100: "32BIT_MACHINE",
    0x0200: "DEBUG_STRIPPED",
    0x0400: "REMOVABLE_RUN_FROM_SWAP",
    0x0800: "NET_RUN_FROM_SWAP",
    0x1000: "SYSTEM",
    0x2000: "DLL",
    0x4000: "UP_SYSTEM_ONLY",
    0x8000: "BYTES_REVERSED_HI",
}

DLL_CHARACTERISTICS_MAP = {
    0x0020: "HIGH_ENTROPY_VA",
    0x0040: "DYNAMIC_BASE (ASLR)",
    0x0080: "FORCE_INTEGRITY_CHECKS",
    0x0100: "NX_COMPAT (DEP)",
    0x0200: "NO_ISOLATION",
    0x0400: "NO_SEH",
    0x0800: "NO_BIND",
    0x1000: "APPCONTAINER",
    0x2000: "WDM_DRIVER",
    0x4000: "GUARD_CF",
    0x8000: "TERMINAL_SERVER_AWARE",
}

KNOWN_DLL_PURPOSES = {
    "MovieMakerLang.dll": "Language/localization resource DLL for Movie Maker UI strings",
    "MovieMakerPreviewClient.dll": "Client-side preview rendering for Movie Maker timeline",
    "WLXMovieLibrary.dll": "Core movie library - manages movie project files and media assets",
    "WLXVideoTrim.dll": "Video trimming/cutting operations for Movie Maker editor",
    "WLXPipeline.dll": "Media processing pipeline - orchestrates rendering/encoding stages",
    "WLXPipetran.dll": "Pipeline transforms - applies effects/transitions during rendering",
    "WLXCodecHost.exe": "Out-of-process codec host for safe codec loading",
    "WLXTranscode.exe": "Out-of-process transcoder for media format conversion",
    "WLXMediaPublishSubscribe.dll": "Media publish/subscribe system for plugin-based sharing",
    "WLMFReadWrite.dll": "Media Foundation read/write layer for media file I/O",
    "WLMFDS.dll": "Media Foundation DirectShow integration layer",
    "WLXMP4Parser.dll": "MP4/MOV container format parser",
    "WLXPhotoBase.dll": "Base photo handling library (shared by Gallery/MovieMaker)",
    "WLXPhotoGallery.exe": "Windows Live Photo Gallery main executable",
    "WLXSlideshow.dll": "Slideshow generation and playback engine",
    "WLXPhotoCinematic.dll": "Cinematic/stylized photo-to-video effects",
    "WLXFaceRecognition.dll": "Face detection and recognition for photo tagging",
}

DIVIDER_THIN = "=" * 120
DIVIDER_THICK = "#" * 120


def format_size(n):
    if n >= 1024 * 1024:
        return f"{n / (1024*1024):.1f} MB"
    elif n >= 1024:
        return f"{n / 1024:.1f} KB"
    return f"{n} B"


def format_flags(flags, flag_map):
    result = []
    for bit, name in sorted(flag_map.items()):
        if flags & bit:
            result.append(name)
    return result


def get_timestamp(pe):
    ts = pe.FILE_HEADER.TimeDateStamp
    if ts:
        import datetime
        try:
            dt = datetime.datetime.fromtimestamp(ts, tz=datetime.timezone.utc)
            return f"{dt.strftime('%Y-%m-%d %H:%M:%S')} UTC (0x{ts:08X})"
        except (OSError, OverflowError, ValueError):
            return f"0x{ts:08X}"
    return "N/A"


def extract_exports(pe):
    exports = []
    if not hasattr(pe, 'DIRECTORY_ENTRY_EXPORT'):
        return exports
    for exp in pe.DIRECTORY_ENTRY_EXPORT.symbols:
        name = exp.name.decode('utf-8', errors='replace') if exp.name else "(ordinal-only)"
        ordinal = exp.ordinal
        entry_rva = f"0x{exp.address:08X}" if exp.address else "N/A"
        fwd = ""
        if exp.forwarder:
            fwd = f" -> {exp.forwarder.decode('utf-8', errors='replace')}"
        exports.append((name, ordinal, entry_rva + fwd))
    return exports


def extract_imports(pe):
    imports = OrderedDict()
    if not hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
        return imports
    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        dll_name = entry.dll.decode('utf-8', errors='replace') if entry.dll else "???"
        funcs = []
        for imp in entry.imports:
            if imp.name:
                fname = imp.name.decode('utf-8', errors='replace')
            else:
                fname = f"Ordinal #{imp.ordinal}"
            funcs.append(fname)
        imports[dll_name] = funcs
    return imports


def extract_sections(pe):
    sections = []
    for sec in pe.sections:
        name = sec.Name.rstrip(b'\x00').decode('utf-8', errors='replace')
        virt_size = sec.Misc_VirtualSize
        raw_size = sec.SizeOfRawData
        virt_addr = sec.VirtualAddress
        chars = sec.Characteristics
        char_flags = []
        if chars & 0x00000020: char_flags.append("CODE")
        if chars & 0x00000040: char_flags.append("INIT_DATA")
        if chars & 0x00000080: char_flags.append("UNINIT_DATA")
        if chars & 0x20000000: char_flags.append("EXEC")
        if chars & 0x40000000: char_flags.append("READ")
        if chars & 0x80000000: char_flags.append("WRITE")
        sections.append({
            'name': name,
            'virt_size': virt_size,
            'raw_size': raw_size,
            'virt_addr': virt_addr,
            'chars': char_flags,
        })
    return sections


def extract_strings_from_pe(pe, min_len=6):
    """Extract ASCII and Unicode strings from all PE sections."""
    ascii_strings = []
    unicode_strings = []

    for section in pe.sections:
        try:
            data = section.get_data()
        except Exception:
            continue

        # ASCII strings
        for match in re.finditer(rb'[\x20-\x7e]{' + str(min_len).encode() + rb',}', data):
            ascii_strings.append(match.group().decode('ascii', errors='replace'))

        # Unicode strings
        for match in re.finditer(rb'(?:[\x20-\x7e]\x00){' + str(min_len).encode() + rb',}', data):
            try:
                s = match.group().decode('utf-16-le', errors='replace')
                unicode_strings.append(s)
            except Exception:
                pass

    return ascii_strings, unicode_strings


def classify_strings(ascii_strings, unicode_strings):
    """Classify extracted strings into categories."""
    all_strings = ascii_strings + unicode_strings
    categories = {
        'rtti_av': [],
        'com_ids': [],
        'error_msgs': [],
        'file_exts': [],
        'dll_deps': [],
        'class_names': [],
        'url_strings': [],
        'guids': [],
        'codec_names': [],
        'media_refs': [],
        'wmf_refs': [],
        'other_notable': [],
    }

    guid_pattern = re.compile(
        r'\{[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}\}'
    )
    rtti_pattern = re.compile(r'\?AV(\w+)')
    com_pattern = re.compile(r'\b[Ii]\w{2,30}\b')
    url_pattern = re.compile(r'https?://[^\s"<>]+')
    ext_pattern = re.compile(r'\.(dll|exe|ax|ocx|mp4|avi|wmv|wma|mp3|jpg|jpeg|png|gif|bmp|tif|tiff|wav|mpg|mpeg|mov|3gp|mfcribbon-ms|wvx|asf|flv|mkv)\b', re.IGNORECASE)
    error_pattern = re.compile(r'(?i)(error|fail|invalid|cannot|unable|exception|abort|fatal|corrupt)', re.IGNORECASE)
    codec_pattern = re.compile(r'(?i)(H\.?264|H\.?265|HEVC|AVC|MPEG-?[24]|VC-?[1N]?|WMV[1-9]|AAC|MP[23]|AC3|FLAC|PCM|WMA|DivX|Xvid|NVENC|Intel|QuickSync)', re.IGNORECASE)
    media_pattern = re.compile(r'(?i)(video|audio|media|stream|encoder|decoder|codec|render|pipeline|transcode|mux|demux|filter|frame|bitrate|sample|format)')
    wmf_pattern = re.compile(r'(?i)(MF_|MediaFoundation|IMF|mfplat|mfreadwrite|mfcreate|dxgi|d3d[0-9]|Direct[23]D)')

    for s in set(all_strings):
        if rtti_pattern.search(s):
            for m in rtti_pattern.finditer(s):
                categories['rtti_av'].append(f"?AV{m.group(1)}")
        if guid_pattern.search(s):
            categories['guids'].extend(guid_pattern.findall(s))
        if url_pattern.search(s):
            categories['url_strings'].extend(url_pattern.findall(s))
        if ext_pattern.search(s):
            categories['file_exts'].extend(ext_pattern.findall(s))
        if error_pattern.search(s):
            if len(s) < 200:
                categories['error_msgs'].append(s)
        if codec_pattern.search(s):
            categories['codec_names'].append(s)
        if media_pattern.search(s):
            if len(s) < 200:
                categories['media_refs'].append(s)
        if wmf_pattern.search(s):
            if len(s) < 200:
                categories['wmf_refs'].append(s)
        if s.startswith('CLSID_') or s.startswith('IID_') or s.startswith('LIBID_'):
            categories['com_ids'].append(s)

    for k in categories:
        categories[k] = list(set(categories[k]))
    return categories


def extract_version_info(pe):
    """Extract version info from PE resources."""
    if not hasattr(pe, 'VS_FIXEDFILEINFO'):
        return None
    vinfo = pe.VS_FIXEDFILEINFO
    if not vinfo:
        return None
    try:
        ms = vinfo[0].StructVersion
        v_major = (vinfo[0].ProductVersionMS >> 16) & 0xFFFF
        v_minor = vinfo[0].ProductVersionMS & 0xFFFF
        v_build = (vinfo[0].ProductVersionLS >> 16) & 0xFFFF
        v_rev = vinfo[0].ProductVersionLS & 0xFFFF
        f_major = (vinfo[0].FileVersionMS >> 16) & 0xFFFF
        f_minor = vinfo[0].FileVersionMS & 0xFFFF
        f_build = (vinfo[0].FileVersionLS >> 16) & 0xFFFF
        f_rev = vinfo[0].FileVersionLS & 0xFFFF
        return {
            'product_version': f"{v_major}.{v_minor}.{v_build}.{v_rev}",
            'file_version': f"{f_major}.{f_minor}.{f_build}.{f_rev}",
        }
    except Exception:
        return None


def analyze_single_file(rel_path):
    """Analyze a single PE file and return results dict."""
    full_path = os.path.join(BASE_DIR, rel_path)
    result = {
        'rel_path': rel_path,
        'full_path': full_path,
        'exists': os.path.exists(full_path),
        'file_size': 0,
        'pe': None,
        'error': None,
    }

    if not result['exists']:
        result['error'] = "File not found"
        return result

    result['file_size'] = os.path.getsize(full_path)

    try:
        pe = pefile.PE(full_path, fast_load=False)
        result['pe'] = pe
    except pefile.PEFormatError as e:
        result['error'] = f"PE parse error: {e}"
        return result
    except Exception as e:
        result['error'] = f"Unexpected error: {e}"
        return result

    return result


def print_pe_header(result):
    pe = result['pe']
    fh = pe.FILE_HEADER
    oh = pe.OPTIONAL_HEADER
    print(f"\n  PE Header Summary:")
    print(f"    Machine:            {MACHINE_MAP.get(fh.Machine, f'Unknown (0x{fh.Machine:04X})')}")
    print(f"    Timestamp:          {get_timestamp(pe)}")
    print(f"    Characteristics:    0x{fh.Characteristics:04X} -> {', '.join(format_flags(fh.Characteristics, CHARACTERISTICS_MAP))}")
    print(f"    DLL Characteristics:0x{oh.DllCharacteristics:04X} -> {', '.join(format_flags(oh.DllCharacteristics, DLL_CHARACTERISTICS_MAP))}")
    print(f"    Subsystem:          {SUBSYSTEM_MAP.get(oh.Subsystem, f'Unknown (0x{oh.Subsystem:04X})')}")
    print(f"    ImageBase:          0x{oh.ImageBase:08X}")
    print(f"    EntryPoint:         0x{oh.AddressOfEntryPoint:08X}")
    print(f"    SectionAlignment:   0x{oh.SectionAlignment:08X}")
    print(f"    FileAlignment:      0x{oh.FileAlignment:08X}")
    print(f"    SizeOfImage:        0x{oh.SizeOfImage:08X} ({format_size(oh.SizeOfImage)})")
    print(f"    SizeOfHeaders:      0x{oh.SizeOfHeaders:08X}")
    num_data_dirs = oh.NumberOfRvaAndSizes
    print(f"    NumberOfRvaAndSizes: {num_data_dirs}")


def print_exports(exports):
    print(f"\n  Exports ({len(exports)} total):")
    if not exports:
        print(f"    (none)")
        return
    for name, ordinal, rva in exports:
        print(f"    Ord #{ordinal:4d}  {rva:28s}  {name}")


def print_imports(imports):
    print(f"\n  Imports ({sum(len(v) for v in imports.values())} functions from {len(imports)} DLLs):")
    if not imports:
        print(f"    (none)")
        return
    for dll, funcs in imports.items():
        print(f"    [{dll}] ({len(funcs)} functions)")
        for f in funcs:
            print(f"      - {f}")


def print_sections(sections):
    print(f"\n  Sections ({len(sections)}):")
    print(f"    {'Name':<12s} {'VirtSize':>12s} {'RawSize':>12s} {'VirtAddr':>12s}  Flags")
    print(f"    {'-'*12} {'-'*12} {'-'*12} {'-'*12}  {'-'*30}")
    for s in sections:
        flags_str = ", ".join(s['chars']) if s['chars'] else "(none)"
        print(f"    {s['name']:<12s} {format_size(s['virt_size']):>12s} {format_size(s['raw_size']):>12s} 0x{s['virt_addr']:08X}  {flags_str}")


def print_strings(categories):
    print(f"\n  String Analysis:")

    if categories['rtti_av']:
        items = sorted(set(categories['rtti_av']))
        print(f"    RTTI ?AV Classes ({len(items)}):")
        for s in items[:80]:
            print(f"      {s}")
        if len(items) > 80:
            print(f"      ... and {len(items)-80} more")

    if categories['guids']:
        items = sorted(set(categories['guids']))
        print(f"    GUIDs ({len(items)}):")
        for s in items[:40]:
            print(f"      {s}")
        if len(items) > 40:
            print(f"      ... and {len(items)-40} more")

    if categories['com_ids']:
        items = sorted(set(categories['com_ids']))
        print(f"    COM Interface/CLSID identifiers ({len(items)}):")
        for s in items[:40]:
            print(f"      {s}")
        if len(items) > 40:
            print(f"      ... and {len(items)-40} more")

    if categories['codec_names']:
        items = sorted(set(categories['codec_names']))
        print(f"    Codec References ({len(items)}):")
        for s in items[:30]:
            print(f"      {s}")

    if categories['media_refs']:
        items = sorted(set(categories['media_refs']))
        print(f"    Media-related Strings ({len(items)}):")
        for s in items[:50]:
            print(f"      {s[:120]}")
        if len(items) > 50:
            print(f"      ... and {len(items)-50} more")

    if categories['wmf_refs']:
        items = sorted(set(categories['wmf_refs']))
        print(f"    Windows Media Foundation / DX References ({len(items)}):")
        for s in items[:40]:
            print(f"      {s[:120]}")
        if len(items) > 40:
            print(f"      ... and {len(items)-40} more")

    if categories['error_msgs']:
        items = sorted(set(categories['error_msgs']))
        print(f"    Error/Failure Messages ({len(items)}):")
        for s in items[:30]:
            print(f"      {s[:120]}")
        if len(items) > 30:
            print(f"      ... and {len(items)-30} more")

    if categories['url_strings']:
        items = sorted(set(categories['url_strings']))
        print(f"    URL Strings ({len(items)}):")
        for s in items[:20]:
            print(f"      {s[:120]}")
        if len(items) > 20:
            print(f"      ... and {len(items)-20} more")


def print_version_info(vinfo):
    print(f"\n  Version Info:")
    if not vinfo:
        print(f"    (none found)")
        return
    print(f"    Product Version:  {vinfo['product_version']}")
    print(f"    File Version:     {vinfo['file_version']}")


def print_purpose(rel_path, exports, imports, ascii_strings, unicode_strings, categories):
    name = os.path.basename(rel_path)
    known = KNOWN_DLL_PURPOSES.get(name, None)

    print(f"\n  Inferred Purpose:")
    if known:
        print(f"    {known}")

    clues = []
    # Analyze imports for clues
    import_dlls_lower = set(d.lower() for d in imports.keys())
    import_dlls = set(imports.keys())
    if 'mfplat.dll' in import_dlls_lower or 'mfreadwrite.dll' in import_dlls_lower:
        clues.append("Uses Windows Media Foundation for media processing")
    if any('d3d' in d for d in import_dlls_lower) or any('dxgi' in d for d in import_dlls_lower):
        clues.append("Uses Direct3D/DXGI for GPU-accelerated processing")
    if 'd2d1.dll' in import_dlls_lower:
        clues.append("Uses Direct2D for 2D rendering")
    if 'windowscodecs.dll' in import_dlls_lower:
        clues.append("Uses Windows Imaging Component (WIC) for image processing")
    if any('msvcr' in d or 'vcruntime' in d for d in import_dlls_lower):
        clues.append("Uses MSVC runtime (C++ native code)")

    mf_dlls = [d for d in import_dlls if 'mf' in d.lower()]
    if mf_dlls:
        clues.append(f"Media Foundation DLLs used: {', '.join(sorted(mf_dlls))}")

    dshow_dlls = [d for d in import_dlls if 'dm' in d.lower() or 'ole' in d.lower() or 'quartz' in d.lower()]
    if dshow_dlls:
        clues.append(f"DirectShow/COM DLLs: {', '.join(sorted(dshow_dlls))}")

    crypto_dlls = [d for d in import_dlls if 'crypt' in d.lower() or 'bcrypt' in d.lower()]
    if crypto_dlls:
        clues.append(f"Cryptography DLLs: {', '.join(sorted(crypto_dlls))}")

    networking = [d for d in import_dlls if 'winhttp' in d.lower() or 'wininet' in d.lower() or d.lower() == 'ws2_32.dll' or 'schannel' in d.lower()]
    if networking:
        clues.append(f"Networking: {', '.join(sorted(networking))}")

    ui_dlls = [d for d in import_dlls if 'user32' in d.lower() or 'gdi32' in d.lower() or 'comctl' in d.lower() or 'dwm' in d.lower() or 'uxtheme' in d.lower()]
    if ui_dlls:
        clues.append(f"UI components: {', '.join(sorted(ui_dlls))}")

    sqlite = [d for d in import_dlls if 'sqlite' in d.lower()]
    if sqlite:
        clues.append("Uses SQLite database")

    for c in clues:
        print(f"    - {c}")

    if not clues and not known:
        print(f"    (unable to determine - further analysis needed)")


def run_analysis():
    print(DIVIDER_THICK)
    print("  MOViemaker Supporting DLLs - Reverse Engineering Analysis")
    print("  Windows Live Essentials 2012")
    print(DIVIDER_THICK)

    all_results = []

    for rel_path in DLL_LIST:
        name = os.path.basename(rel_path)
        print(f"\n{DIVIDER_THIN}")
        print(f"  ANALYZING: {rel_path}")
        file_size = os.path.getsize(os.path.join(BASE_DIR, rel_path))
        print(f"  File Size: {format_size(file_size)} ({file_size:,} bytes)")
        print(DIVIDER_THIN)

        result = analyze_single_file(rel_path)

        if result['error']:
            print(f"\n  ERROR: {result['error']}")
            all_results.append({
                'name': name,
                'rel_path': rel_path,
                'size': result['file_size'],
                'is_dll': True,
                'num_exports': 0,
                'num_imports': 0,
                'key_deps': [],
                'purpose': f"Error: {result['error']}",
                'error': result['error'],
            })
            continue

        pe = result['pe']
        is_dll = hasattr(pe, 'OPTIONAL_HEADER') and (pe.FILE_HEADER.Characteristics & 0x2000) != 0
        is_exe = (pe.FILE_HEADER.Characteristics & 0x0002) != 0

        print_pe_header(result)

        exports = extract_exports(pe)
        print_exports(exports)

        imports = extract_imports(pe)
        print_imports(imports)

        sections = extract_sections(pe)
        print_sections(sections)

        ascii_strings, unicode_strings = extract_strings_from_pe(pe)
        categories = classify_strings(ascii_strings, unicode_strings)
        print_strings(categories)

        vinfo = extract_version_info(pe)
        print_version_info(vinfo)

        print_purpose(rel_path, exports, imports, ascii_strings, unicode_strings, categories)

        key_deps = []
        for dll_name in imports:
            lower = dll_name.lower()
            if lower not in ('kernel32.dll', 'ntdll.dll', 'user32.dll', 'advapi32.dll',
                             'msvcrt.dll', 'msvcr100.dll', 'msvcr110.dll', 'msvcr120.dll',
                             'vcruntime140.dll', 'api-ms-win-core', 'api-ms-win-crt',
                             'shell32.dll', 'ole32.dll', 'oleaut32.dll', 'gdi32.dll',
                             'comctl32.dll', 'comdlg32.dll', 'shlwapi.dll', 'version.dll',
                             'ws2_32.dll', 'rpcrt4.dll', 'sechost.dll', 'crypt32.dll',
                             'ncrypt.dll', 'bcrypt.dll', 'wintab32.dll', 'usp10.dll',
                             'imm32.dll', 'winmm.dll', 'cryptsp.dll', 'wintrust.dll',
                             'profapi.dll', 'propsys.dll', 'clbcatq.dll', 'sxs.dll',
                             'apphelp.dll', 'dwmapi.dll', 'dbghelp.dll', 'imagehlp.dll',
                             'setupapi.dll', 'bcryptprimitives.dll', 'ntmarta.dll',
                             'devobj.dll', 'cfgmgr32.dll', 'kernelbase.dll',
                             'api-ms-win-downlevel', 'schannel.dll', 'wtsapi32.dll',
                             'netapi32.dll', 'IPHLPAPI.dll', 'rasapi32.dll',
                             'cryptbase.dll', 'sspicli.dll', 'combase.dll'):
                key_deps.append(dll_name)

        purpose = KNOWN_DLL_PURPOSES.get(name, "Unknown")
        all_results.append({
            'name': name,
            'rel_path': rel_path,
            'size': result['file_size'],
            'is_dll': is_dll,
            'is_exe': is_exe,
            'machine': MACHINE_MAP.get(pe.FILE_HEADER.Machine, f"0x{pe.FILE_HEADER.Machine:04X}"),
            'num_exports': len(exports),
            'num_imports': sum(len(v) for v in imports.values()),
            'num_import_dlls': len(imports),
            'num_sections': len(sections),
            'key_deps': key_deps,
            'purpose': purpose,
            'version': vinfo,
            'error': None,
        })

        pe.close()

    # Summary table
    print(f"\n\n{DIVIDER_THICK}")
    print("  SUMMARY TABLE")
    print(DIVIDER_THICK)

    header = f"{'DLL Name':<40s} {'Size':>10s} {'Type':>5s} {'Exp':>5s} {'Imp':>6s} {'#DLLs':>5s} {'Machine':>14s}  Key Dependencies"
    print(header)
    print("-" * len(header))

    for r in all_results:
        dll_type = "DLL" if r.get('is_dll') else "EXE"
        key_dep_str = ", ".join(r['key_deps'][:5])
        if len(r['key_deps']) > 5:
            key_dep_str += f" +{len(r['key_deps'])-5} more"
        machine = r.get('machine', 'N/A')
        print(f"{r['name']:<40s} {format_size(r['size']):>10s} {dll_type:>5s} {r['num_exports']:>5d} {r['num_imports']:>6d} {r.get('num_import_dlls',0):>5d} {machine:>14s}  {key_dep_str}")

    print(f"\n\n  Purpose Summary:")
    for r in all_results:
        print(f"    {r['name']:<40s} {r['purpose']}")

    if any(r.get('version') for r in all_results):
        print(f"\n  Version Info:")
        for r in all_results:
            if r.get('version'):
                print(f"    {r['name']:<40s} File: {r['version']['file_version']}  Product: {r['version']['product_version']}")

    print(f"\n\n{DIVIDER_THICK}")
    print("  Analysis complete.")
    print(DIVIDER_THICK)


if __name__ == "__main__":
    run_analysis()
