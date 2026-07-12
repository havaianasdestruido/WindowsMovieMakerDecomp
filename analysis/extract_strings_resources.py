#!/usr/bin/env python3
"""
Extract ALL strings and resources from Windows Movie Maker binaries.
Uses pefile to parse PE structure and extract:
  - RT_STRING tables (UI strings, error messages, dialog text)
  - RT_DIALOG resources (UI layouts)
  - RT_MENU resources
  - RT_ACCELERATOR resources
  - RT_VERSION resources (VS_VERSION_INFO, StringFileInfo)
  - DirectUI .duxt resources
  - HLSL shader resources
  - ASCII and UTF-16LE strings from .text/.data sections
  - Categorized strings (extensions, registry, COM, errors, APIs, URLs, paths, UI)
"""

import os
import re
import sys
import struct
import collections

try:
    import pefile
except ImportError:
    print("ERROR: pefile not installed. Run: pip install pefile")
    sys.exit(1)

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
BINARIES = [
    r"C:\Users\mcmco\Desktop\WMMR\undecomp\Photo Gallery\MovieMaker.exe",
    r"C:\Users\mcmco\Desktop\WMMR\undecomp\Photo Gallery\MovieMakerCore.dll",
    r"C:\Users\mcmco\Desktop\WMMR\undecomp\Photo Gallery\MovieMakerLang.dll",
    r"C:\Users\mcmco\Desktop\WMMR\undecomp\Shared\WLXPhotoBase.dll",
]

MIN_STRING_LEN = 4

# Resource type IDs we care about
RT_CURSOR       = 1
RT_BITMAP       = 2
RT_ICON         = 3
RT_MENU         = 4
RT_DIALOG       = 5
RT_STRING       = 6
RT_FONTDIR      = 7
RT_FONT         = 8
RT_ACCELERATOR  = 9
RT_RCDATA       = 10
RT_MESSAGETABLE = 11
RT_GROUP_CURSOR = 12
RT_GROUP_ICON   = 14
RT_VERSION      = 16
RT_DLGINCLUDE   = 17
RT_PLUGPLAY     = 19
RT_VXD          = 20
RT_ANICURSOR    = 21
RT_ANIICON      = 22
RT_HTML         = 23
RT_MANIFEST     = 24

RT_NAMES = {
    RT_CURSOR: "RT_CURSOR", RT_BITMAP: "RT_BITMAP", RT_ICON: "RT_ICON",
    RT_MENU: "RT_MENU", RT_DIALOG: "RT_DIALOG", RT_STRING: "RT_STRING",
    RT_FONTDIR: "RT_FONTDIR", RT_FONT: "RT_FONT", RT_ACCELERATOR: "RT_ACCELERATOR",
    RT_RCDATA: "RT_RCDATA", RT_MESSAGETABLE: "RT_MESSAGETABLE",
    RT_GROUP_CURSOR: "RT_GROUP_CURSOR", RT_GROUP_ICON: "RT_GROUP_ICON",
    RT_VERSION: "RT_VERSION", RT_DLGINCLUDE: "RT_DLGINCLUDE",
    RT_PLUGPLAY: "RT_PLUGPLAY", RT_VXD: "RT_VXD",
    RT_ANICURSOR: "RT_ANICURSOR", RT_ANIICON: "RT_ANIICON",
    RT_HTML: "RT_HTML", RT_MANIFEST: "RT_MANIFEST",
}

# Known resource section names we might encounter for DirectUI / HLSL
KNOWN_CUSTOM_RCDATA_NAMES = {
    "DUXT": "DirectUI",
    "duxt": "DirectUI",
    "HLSL": "HLSL Shader",
    "hlsl": "HLSL Shader",
    "SHADER": "HLSL Shader",
    "shader": "HLSL Shader",
}


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
def safe_decode(data):
    """Try UTF-8 then Latin-1."""
    try:
        return data.decode("utf-8")
    except Exception:
        return data.decode("latin-1", errors="replace")


def safe_print(s):
    """Print string, replacing non-encodable chars for cp1252 console."""
    if isinstance(s, str):
        print(s.encode("cp1252", errors="replace").decode("cp1252"))
    else:
        print(s)


def extract_ascii_strings(data, min_len=MIN_STRING_LEN):
    """Extract printable ASCII strings (min_len chars)."""
    pattern = re.compile(rb'[\x20-\x7e]{%d,}' % min_len)
    return [m.group().decode("ascii") for m in pattern.finditer(data)]


def extract_utf16le_strings(data, min_len=MIN_STRING_LEN):
    """Extract UTF-16LE strings (min_len chars)."""
    # Each char is 2 bytes; we need min_len*2 bytes of valid wchar pattern
    pattern = re.compile(
        rb'(?:[\x20-\x7e]\x00){%d,}' % min_len
    )
    results = []
    for m in pattern.finditer(data):
        try:
            s = m.group().decode("utf-16-le")
            results.append(s)
        except Exception:
            pass
    return results


def get_section_data(pe, section_name):
    """Get raw data for a named section, or empty bytes."""
    for sec in pe.sections:
        name = sec.Name.rstrip(b'\x00').decode('ascii', errors='replace')
        if name == section_name:
            return sec.get_data()
    return b''


def get_all_section_data(pe, names=None):
    """Concatenate data from multiple sections."""
    if names is None:
        names = ['.text', '.data', '.rdata', '.idata', '.edata']
    buf = b''
    for sec in pe.sections:
        name = sec.Name.rstrip(b'\x00').decode('ascii', errors='replace')
        if names is None or name in names:
            buf += sec.get_data()
    return buf


# ---------------------------------------------------------------------------
# RT_STRING table parser
# ---------------------------------------------------------------------------

def parse_rt_string_table(data):
    """
    Parse RT_STRING resource data.
    Format: array of 16-string blocks. Each block:
      - 16 x uint16 length (in UTF-16 chars), 0 means empty
      - followed by that many UTF-16LE chars per non-zero entry
    Returns dict of {string_id: string_value}.
    """
    strings = {}
    offset = 0
    block_id = 0  # each block holds IDs block_id*16 .. block_id*16+15

    while offset + 32 <= len(data):
        lengths = struct.unpack_from('<16H', data, offset)
        offset += 32
        for i, length in enumerate(lengths):
            if length > 0:
                byte_count = length * 2
                if offset + byte_count <= len(data):
                    raw = data[offset:offset + byte_count]
                    try:
                        s = raw.decode("utf-16-le")
                    except Exception:
                        s = raw.hex()
                    string_id = block_id * 16 + i
                    strings[string_id] = s
                    offset += byte_count
                else:
                    offset += byte_count
        block_id += 1

        # Safety: if we've consumed everything, stop
        if offset >= len(data):
            break

    return strings


# ---------------------------------------------------------------------------
# RT_DIALOG parser (minimal - dump as hex + extract any string data)
# ---------------------------------------------------------------------------

def parse_rt_dialog(data):
    """
    Parse RT_DIALOG resource to extract embedded strings.
    A dialog template starts with:
      DLG_TEMPLATE: style(uint32), dwExtendedStyle(uint32), cdit(uint16),
                     x(int16), y(int16), cx(int16), cy(int16),
                     menu(str), class(str), title(str), pointsize(uint16),
                     weight(uint16), italic(uint8), charset(uint8), font(str)
    Then cdit x DLG_ITEM_TEMPLATE entries.
    """
    result = {"raw_size": len(data), "strings": [], "controls": []}
    if len(data) < 18:
        return result

    try:
        offset = 0
        style = struct.unpack_from('<I', data, offset)[0]; offset += 4
        extended_style = struct.unpack_from('<I', data, offset)[0]; offset += 4
        num_controls = struct.unpack_from('<H', data, offset)[0]; offset += 2
        x = struct.unpack_from('<h', data, offset)[0]; offset += 2
        y = struct.unpack_from('<h', data, offset)[0]; offset += 2
        cx = struct.unpack_from('<h', data, offset)[0]; offset += 2
        cy = struct.unpack_from('<h', data, offset)[0]; offset += 2

        result["style"] = style
        result["extended_style"] = extended_style
        result["num_controls"] = num_controls
        result["rect"] = (x, y, cx, cy)

        def read_prefixed_string_or_ordinal(d, off):
            if off + 2 > len(d):
                return "", off
            val = struct.unpack_from('<H', d, off)[0]
            if val == 0xFFFF:
                # ordinal
                if off + 4 <= len(d):
                    ord_val = struct.unpack_from('<H', d, off + 2)[0]
                    return f"#{ord_val}", off + 4
                return "", off + 2
            elif val == 0:
                return "", off + 2
            else:
                # zero-terminated UTF-16LE string
                end = off
                while end + 2 <= len(d) and struct.unpack_from('<H', d, end)[0] != 0:
                    end += 2
                end += 2  # include null
                try:
                    s = d[off:end].decode("utf-16-le")
                except Exception:
                    s = d[off:end].hex()
                return s, end

        # menu
        menu_str, offset = read_prefixed_string_or_ordinal(data, offset)
        # class
        class_str, offset = read_prefixed_string_or_ordinal(data, offset)
        # title
        title_str, offset = read_prefixed_string_or_ordinal(data, offset)
        if title_str:
            result["strings"].append(title_str)

        # font info (only if DS_SETFONT in style, bit 0x40)
        if style & 0x40:
            if offset + 8 <= len(data):
                pointsize = struct.unpack_from('<H', data, offset)[0]; offset += 2
                weight = struct.unpack_from('<H', data, offset)[0]; offset += 2
                italic = data[offset]; offset += 1
                charset = data[offset]; offset += 1
                font_str, offset = read_prefixed_string_or_ordinal(data, offset)
                if font_str:
                    result["strings"].append(font_str)
                    result["font"] = font_str

        # Parse individual control templates
        for ctrl_idx in range(num_controls):
            if offset + 18 > len(data):
                break
            # DLG_ITEM_TEMPLATE is 4-byte aligned
            offset = (offset + 3) & ~3
            if offset + 18 > len(data):
                break
            ctrl_style = struct.unpack_from('<I', data, offset)[0]; offset += 4
            ctrl_ext = struct.unpack_from('<I', data, offset)[0]; offset += 4
            ctrl_x = struct.unpack_from('<h', data, offset)[0]; offset += 2
            ctrl_y = struct.unpack_from('<h', data, offset)[0]; offset += 2
            ctrl_cx = struct.unpack_from('<h', data, offset)[0]; offset += 2
            ctrl_cy = struct.unpack_from('<h', data, offset)[0]; offset += 2
            ctrl_id = struct.unpack_from('<H', data, offset)[0]; offset += 2

            ctrl_class, offset = read_prefixed_string_or_ordinal(data, offset)
            ctrl_text, offset = read_prefixed_string_or_ordinal(data, offset)

            if offset < len(data):
                offset += 1  # extraData byte

            ctrl_info = {
                "id": ctrl_id,
                "class": ctrl_class,
                "text": ctrl_text,
                "rect": (ctrl_x, ctrl_y, ctrl_cx, ctrl_cy),
            }
            result["controls"].append(ctrl_info)
            if ctrl_text:
                result["strings"].append(ctrl_text)

    except Exception as e:
        result["error"] = str(e)

    return result


# ---------------------------------------------------------------------------
# RT_MENU parser
# ---------------------------------------------------------------------------

def parse_rt_menu(data):
    """Parse RT_MENU resource to extract menu item strings."""
    result = {"raw_size": len(data), "items": []}
    if len(data) < 4:
        return result

    def read_wstring(d, off):
        if off + 2 > len(d):
            return "", off
        val = struct.unpack_from('<H', d, off)[0]
        if val == 0:
            return "", off + 2
        end = off
        while end + 2 <= len(d) and struct.unpack_from('<H', d, end)[0] != 0:
            end += 2
        end += 2
        try:
            return d[off:end].decode("utf-16-le"), end
        except Exception:
            return d[off:end].hex(), off + 2

    try:
        offset = 0
        menu_ver = struct.unpack_from('<H', data, offset)[0]; offset += 2
        menu_help = struct.unpack_from('<H', data, offset)[0]; offset += 2

        result["version"] = menu_ver
        result["help_id"] = menu_help

        while offset + 4 <= len(data):
            # Each item: mtOption(uint32) then mtString(zero-terminated UTF-16LE)
            # If mtString is empty (starts with 0x0000), it's a popup end marker
            mt_option = struct.unpack_from('<I', data, offset)[0]; offset += 4

            text, offset = read_wstring(data, offset)
            if text:
                result["items"].append({
                    "flags": mt_option,
                    "text": text,
                })
            elif mt_option == 0:
                break  # end of menu
    except Exception as e:
        result["error"] = str(e)

    return result


# ---------------------------------------------------------------------------
# RT_ACCELERATOR parser
# ---------------------------------------------------------------------------

def parse_rt_accelerator(data):
    """Parse RT_ACCELERATOR resource."""
    result = {"raw_size": len(data), "entries": []}
    offset = 0
    try:
        while offset + 8 <= len(data):
            fFlags = struct.unpack_from('<H', data, offset)[0]; offset += 2
            anVirt = struct.unpack_from('<H', data, offset)[0]; offset += 2
            anID = struct.unpack_from('<H', data, offset)[0]; offset += 2
            # padding to 4 bytes
            offset = (offset + 3) & ~3
            result["entries"].append({
                "flags": fFlags,
                "key": anVirt,
                "id": anID,
            })
            if fFlags & 0x80:  # FEND
                break
    except Exception as e:
        result["error"] = str(e)
    return result


# ---------------------------------------------------------------------------
# RT_VERSION parser
# ---------------------------------------------------------------------------

def parse_rt_version(data):
    """Parse RT_VERSION resource (VS_VERSION_INFO)."""
    result = {"raw_size": len(data), "version_info": {}, "string_file_info": [], "var_file_info": {}}
    if len(data) < 34:
        return result

    def read_wstring_at(d, off):
        length = struct.unpack_from('<H', d, off)[0]
        if length == 0:
            return "", off + 2
        byte_count = length * 2
        if off + 2 + byte_count > len(d):
            return "", off + 2 + byte_count
        try:
            return d[off + 2:off + 2 + byte_count].decode("utf-16-le"), off + 2 + byte_count
        except Exception:
            return d[off + 2:off + 2 + byte_count].hex(), off + 2 + byte_count

    try:
        # Top-level VS_VERSION_INFO
        length = struct.unpack_from('<H', data, 0)[0]
        val_length = struct.unpack_from('<H', data, 2)[0]
        type_ = struct.unpack_from('<H', data, 4)[0]

        key, offset = read_wstring_at(data, 6)
        # Align to 4 bytes
        offset = (offset + 3) & ~3

        result["key"] = key

        if offset + val_length <= len(data):
            val_data = data[offset:offset + val_length]
            if len(val_data) >= 48:
                vs_magic = struct.unpack_from('<I', val_data, 0)[0]
                if vs_magic == 0xFEEF04BD:
                    dwFileVersionMS = struct.unpack_from('<I', val_data, 4)[0]
                    dwFileVersionLS = struct.unpack_from('<I', val_data, 8)[0]
                    dwProductVersionMS = struct.unpack_from('<I', val_data, 12)[0]
                    dwProductVersionLS = struct.unpack_from('<I', val_data, 16)[0]
                    dwFileFlagsMask = struct.unpack_from('<I', val_data, 20)[0]
                    dwFileFlags = struct.unpack_from('<I', val_data, 24)[0]
                    dwFileOS = struct.unpack_from('<I', val_data, 28)[0]
                    dwFileType = struct.unpack_from('<I', val_data, 32)[0]
                    dwFileSubtype = struct.unpack_from('<I', val_data, 36)[0]
                    dwFileDateMS = struct.unpack_from('<I', val_data, 40)[0]
                    dwFileDateLS = struct.unpack_from('<I', val_data, 44)[0]

                    file_ver = "%d.%d.%d.%d" % (
                        (dwFileVersionMS >> 16) & 0xFFFF,
                        dwFileVersionMS & 0xFFFF,
                        (dwFileVersionLS >> 16) & 0xFFFF,
                        dwFileVersionLS & 0xFFFF,
                    )
                    prod_ver = "%d.%d.%d.%d" % (
                        (dwProductVersionMS >> 16) & 0xFFFF,
                        dwProductVersionMS & 0xFFFF,
                        (dwProductVersionLS >> 16) & 0xFFFF,
                        dwProductVersionLS & 0xFFFF,
                    )
                    result["version_info"] = {
                        "file_version": file_ver,
                        "product_version": prod_ver,
                        "file_flags": dwFileFlags,
                        "file_os": dwFileOS,
                        "file_type": dwFileType,
                        "file_subtype": dwFileSubtype,
                    }

        # Now parse children (StringFileInfo and VarFileInfo)
        # After the VS_VERSION_INFO value, aligned to 4 bytes, children follow
        offset += val_length
        offset = (offset + 3) & ~3

        while offset + 8 <= len(data):
            child_length = struct.unpack_from('<H', data, offset)[0]
            if child_length == 0:
                break
            child_val_length = struct.unpack_from('<H', data, offset + 2)[0]
            child_type = struct.unpack_from('<H', data, offset + 4)[0]

            child_key, child_key_end = read_wstring_at(data, offset + 6)
            child_key_end = (child_key_end + 3) & ~3

            child_val_offset = offset + child_key_end

            if child_key == "StringFileInfo":
                # Parse string table children
                st_offset = child_val_offset
                end_of_child = offset + child_length
                while st_offset + 8 <= end_of_child and st_offset + 8 <= len(data):
                    st_len = struct.unpack_from('<H', data, st_offset)[0]
                    if st_len == 0:
                        break
                    st_val_len = struct.unpack_from('<H', data, st_offset + 2)[0]
                    st_type = struct.unpack_from('<H', data, st_offset + 4)[0]
                    st_key, st_key_end = read_wstring_at(data, st_offset + 6)
                    st_key_end = (st_key_end + 3) & ~3
                    st_val_offset = st_offset + st_key_end

                    # Parse individual strings
                    str_offset = st_val_offset
                    str_end = st_offset + st_len
                    strings = {}
                    while str_offset + 8 <= str_end and str_offset + 8 <= len(data):
                        s_len = struct.unpack_from('<H', data, str_offset)[0]
                        if s_len == 0:
                            break
                        s_val_len = struct.unpack_from('<H', data, str_offset + 2)[0]
                        s_type = struct.unpack_from('<H', data, str_offset + 4)[0]
                        s_key, s_key_end = read_wstring_at(data, str_offset + 6)
                        s_key_end = (s_key_end + 3) & ~3
                        s_val_off = str_offset + s_key_end
                        s_val, _ = read_wstring_at(data, s_val_off)
                        strings[s_key] = s_val
                        str_offset += s_len
                        str_offset = (str_offset + 3) & ~3

                    result["string_file_info"].append({
                        "lang": st_key,
                        "strings": strings,
                    })
                    st_offset += st_len
                    st_offset = (st_offset + 3) & ~3

            elif child_key == "VarFileInfo":
                # Variable file info - just record key/value pairs
                var_offset = child_val_offset
                end_of_child = offset + child_length
                while var_offset + 8 <= end_of_child and var_offset + 8 <= len(data):
                    v_len = struct.unpack_from('<H', data, var_offset)[0]
                    if v_len == 0:
                        break
                    v_val_len = struct.unpack_from('<H', data, var_offset + 2)[0]
                    v_type = struct.unpack_from('<H', data, var_offset + 4)[0]
                    v_key, v_key_end = read_wstring_at(data, var_offset + 6)
                    v_key_end = (v_key_end + 3) & ~3
                    v_val_off = var_offset + v_key_end
                    # Read as bytes (VarFileInfo values are often binary)
                    if v_val_len > 0 and v_val_off + v_val_len <= len(data):
                        val_bytes = data[v_val_off:v_val_off + v_val_len]
                        # If it looks like a Translation struct (pairs of uint16)
                        if v_key == "Translation" and v_val_len >= 4:
                            trans = []
                            for ti in range(0, v_val_len, 4):
                                if ti + 4 <= v_val_len:
                                    lang_id = struct.unpack_from('<H', val_bytes, ti)[0]
                                    cp_id = struct.unpack_from('<H', val_bytes, ti + 2)[0]
                                    trans.append({"lang": lang_id, "codepage": cp_id})
                            result["var_file_info"][v_key] = trans
                        else:
                            result["var_file_info"][v_key] = val_bytes.hex()
                    var_offset += v_len
                    var_offset = (var_offset + 3) & ~3

            offset += child_length
            offset = (offset + 3) & ~3

    except Exception as e:
        result["error"] = str(e)

    return result


# ---------------------------------------------------------------------------
# String categorization
# ---------------------------------------------------------------------------

def categorize_strings(strings):
    """Group strings into categories."""
    categories = {
        "file_extensions": [],
        "registry_paths": [],
        "com_clsid_iid": [],
        "error_messages": [],
        "api_function_names": [],
        "ui_labels": [],
        "urls": [],
        "file_paths": [],
        "other": [],
    }

    ext_re = re.compile(r'\.(mp4|avi|wmv|asf|wma|mp3|wav|jpg|jpeg|png|gif|bmp|tiff|tif|mpg|mpeg|mpe|vob|mkv|mov|flv|3gp|3g2|divx|ogv|webm|m4v|m2ts|mts|ts|mod|tod|dvr-ms|wdp|jxr|hdp|heic|heif|avchd|fx|wlmp|wlav|wlmp|smil|sami|sbk|aif|aiff|mid|midi|rmi|au|snd|cda|aac|flac|alac|wve|ism|isml|mp4v|mp2v|dpg|gif|icon|ico|cur|ani|dds|exr|hdr|pfm|tga|wdp|xpm|xbm|pbm|pgm|ppm|pnm|ras|rgb|rgba|sgi|sun|tiff?x?|vtf|webp)', re.IGNORECASE)

    registry_re = re.compile(r'(HKEY_|HKLM|HKCU|HKCR|HKU|HKCC|Software\\\\|SYSTEM\\\\|CurrentControlSet|Microsoft\\\\|Windows\\\\)', re.IGNORECASE)

    com_re = re.compile(r'(\{[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}\}|I[A-Z][a-zA-Z0-9]{2,30})')

    error_kw_re = re.compile(r'(error|fail|invalid|cannot|unable|exception|abort|fatal|denied|not found|corrupt|timeout|overflow|underflow|out of memory|access violation|permission|denied|warning)', re.IGNORECASE)

    api_re = re.compile(r'^(CoCreate|CoInitialize|SHGet|CreateFile|ReadFile|WriteFile|CloseHandle|LoadLibrary|GetProcAddress|FreeLibrary|RegOpen|RegQuery|RegClose|SetWindow|GetWindow|PostMessage|SendMessage|CreateWindow|DestroyWindow|ShowWindow|SetTimer|KillTimer|HeapAlloc|HeapFree|VirtualAlloc|VirtualFree|GlobalAlloc|GlobalFree|LocalAlloc|LocalFree|MultiByteToWideChar|WideCharToMultiByte|StringCch|PathCombine|PathAppend|SHCreate|SHFile|CoTaskMem|IMalloc|IStream|ISequentialStream|IPropertyStore|IClassFactory|IUnknown|IMediaEvent|IMediaControl|IGraphBuilder|IVideoWindow|IBasicAudio|IBasicVideo|IMediaSeeking|IMediaPosition|IFilterGraph|IFilterGraph2|IGraphBuilder2|IGraphConfig|ISpecifyPropertyPages|CreateMediaType|FreeMediaType|AMGetErrorText|FindFirstFile|FindNextFile|FindClose|GetFileAttributes|SetFileAttributes|GetFullPathName|GetTempPath|GetTempFileName|MoveFile|CopyFile|DeleteFile|CreateDirectory|RemoveDirectory|GetCurrentDirectory|SetCurrentDirectory|GetModuleFileName|GetModuleHandle|GetSystemDirectory|GetWindowsDirectory|GetEnvironmentVariable|SetEnvironmentVariable|FormatMessage|GetLastError|SetLastError|InitializeCriticalSection|EnterCriticalSection|LeaveCriticalSection|DeleteCriticalSection|CreateEvent|SetEvent|ResetEvent|WaitForSingleObject|CreateThread|ExitThread|GetCurrentThreadId|InterlockedIncrement|InterlockedDecrement|InterlockedExchange|InterlockedCompareExchange|InterlockedExchangeAdd)', re.IGNORECASE)

    url_re = re.compile(r'(https?://[^\s<>\"\']+|www\.[^\s<>\"\']+|ftp://[^\s<>\"\']+|mailto:[^\s<>\"\']+)', re.IGNORECASE)

    path_re = re.compile(r'([A-Z]:\\\\[^\s\"<>]+|\\\\\\\\[^\s\"<>]+|%[A-Za-z]+%\\\\[^\s\"<>]+|%[A-Za-z]+%\\\\[^\s\"<>]+|C:\\\\[^\s\"<>]+|D:\\\\[^\s\"<>]+|\.\\\\[^\s\"<>]+)')

    for s in strings:
        categorized = False

        if ext_re.search(s):
            categories["file_extensions"].append(s)
            categorized = True
        if registry_re.search(s):
            categories["registry_paths"].append(s)
            categorized = True
        if com_re.search(s):
            categories["com_clsid_iid"].append(s)
            categorized = True
        if error_kw_re.search(s):
            categories["error_messages"].append(s)
            categorized = True
        if api_re.search(s):
            categories["api_function_names"].append(s)
            categorized = True
        if url_re.search(s):
            categories["urls"].append(s)
            categorized = True
        if path_re.search(s):
            categories["file_paths"].append(s)
            categorized = True

        # If it looks like a short UI string (sentence case, spaces, <=200 chars,
        # no path separators, no special chars)
        if not categorized:
            if (len(s) <= 200 and ' ' in s and not s.startswith(('0x', '\\', '/'))
                    and not s.startswith('{') and not re.match(r'^[\d\s\.\-:,]+$', s)):
                categories["ui_labels"].append(s)
                categorized = True

        if not categorized:
            categories["other"].append(s)

    return categories


# ---------------------------------------------------------------------------
# Main extraction for a single binary
# ---------------------------------------------------------------------------

def analyze_binary(filepath):
    """Full analysis of a single PE binary."""
    basename = os.path.basename(filepath)
    size_kb = os.path.getsize(filepath) / 1024

    print("\n" + "=" * 100)
    print(f"BINARY: {basename}  ({size_kb:.1f} KB)")
    print("=" * 100)

    try:
        pe = pefile.PE(filepath, fast_load=False)
    except Exception as e:
        print(f"  ERROR loading PE: {e}")
        return

    # --- Basic PE info ---
    print(f"\n  Machine: 0x{pe.FILE_HEADER.Machine:04X}")
    print(f"  Sections: {[s.Name.decode('ascii', errors='replace').rstrip('\\x00') for s in pe.sections]}")
    print(f"  Number of resources: {pe.OPTIONAL_HEADER.NumberOfRvaAndSizes}")

    # --- Section sizes ---
    print("\n  --- Section Layout ---")
    for sec in pe.sections:
        name = sec.Name.decode('ascii', errors='replace').rstrip('\x00')
        print(f"  {name:12s}  VA=0x{sec.VirtualAddress:08X}  Size=0x{sec.SizeOfRawData:08X}  "
              f"Chars=0x{sec.Characteristics:08X}")

    # =====================================================================
    # PART 1: RESOURCE EXTRACTION
    # =====================================================================
    print("\n" + "-" * 80)
    print("  PART 1: RESOURCE EXTRACTION")
    print("-" * 80)

    if not hasattr(pe, 'DIRECTORY_ENTRY_RESOURCE') or pe.DIRECTORY_ENTRY_RESOURCE is None:
        print("  [!] No resource directory found. Trying to parse resources...")
        try:
            pe.parse_resources_direct()
        except Exception as e:
            print(f"  [!] Could not parse resources: {e}")

    if not hasattr(pe, 'DIRECTORY_ENTRY_RESOURCE') or pe.DIRECTORY_ENTRY_RESOURCE is None:
        print("  [!] No resources in this binary.")
    else:
        resource_dir = pe.DIRECTORY_ENTRY_RESOURCE

        def print_resource_tree(entry, depth=0, path=""):
            """Recursively print resource tree and collect data."""
            prefix = "  " * (depth + 2)
            if hasattr(entry, 'struct'):
                rt = entry.struct.Id
                if rt is None and hasattr(entry, 'Name') and entry.Name:
                    rt_name = entry.Name.string.decode('ascii', errors='replace') if isinstance(entry.Name.string, bytes) else str(entry.Name)
                else:
                    rt_name = RT_NAMES.get(rt, f"Unknown({rt})" if rt else "Dir")
                print(f"{prefix}[{rt_name}]")
            if hasattr(entry, 'directory') and entry.directory:
                for e2 in entry.directory.entries:
                    print_resource_tree(e2, depth + 1, path)
            elif hasattr(entry, 'data') and entry.data:
                data = pe.get_data(
                    entry.data.struct.OffsetToData,
                    entry.data.struct.Size
                )
                print(f"{prefix}  Data @ 0x{entry.data.struct.OffsetToData:08X}, Size={len(data)} bytes")
                return (entry, data)
            return None

        # Walk and collect all resources by type
        resources_by_type = collections.defaultdict(list)

        def collect_resources(entry, depth=0):
            """Collect resource data grouped by type."""
            if hasattr(entry, 'directory') and entry.directory:
                for e2 in entry.directory.entries:
                    collect_resources(e2, depth + 1)
            elif hasattr(entry, 'data') and entry.data:
                data = pe.get_data(
                    entry.data.struct.OffsetToData,
                    entry.data.struct.Size
                )
                # Determine the resource type from the parent entries
                # We need to walk back up - simpler: just collect all
                resources_by_type[entry.data.struct.OffsetToData].append(data)

        # Better approach: walk the resource directory structure properly
        def walk_resource_dir(directory, type_id=None, lang_id=None, name=None):
            """Walk resource dir, yielding (type_id, name, lang_id, data)."""
            for entry in directory.entries:
                if entry.name is not None:
                    entry_name = entry.name.string.decode('utf-8', errors='replace') if isinstance(entry.name.string, bytes) else str(entry.name)
                else:
                    entry_name = entry.struct.Id

                if hasattr(entry, 'directory') and entry.directory:
                    yield from walk_resource_dir(entry.directory, entry_name if type_id is None else type_id, name if lang_id is None else lang_id, entry_name)
                elif hasattr(entry, 'data') and entry.data:
                    data = pe.get_data(entry.data.struct.OffsetToData, entry.data.struct.Size)
                    yield (type_id, name, entry_name, data)

        # Collect all resources
        all_resources = list(walk_resource_dir(resource_dir))
        print(f"\n  Total resources found: {len(all_resources)}")

        # Group by type
        type_groups = collections.defaultdict(list)
        for type_id, lang, name, data in all_resources:
            type_groups[type_id].append((name, lang, data))

        print("  Resource types present:")
        for type_id, items in sorted(type_groups.items(), key=lambda x: str(x[0])):
            type_name = RT_NAMES.get(type_id, str(type_id))
            print(f"    {type_name} ({type_id}): {len(items)} resource(s)")

        # --- RT_STRING extraction ---
        print("\n  --- RT_STRING Tables (UI Strings) ---")
        for type_id in [RT_STRING, "RT_STRING"]:
            if type_id in type_groups:
                for name, lang, data in type_groups[type_id]:
                    block_name = f"name={name}" if name else "unnamed"
                    print(f"\n  [RT_STRING] {block_name}, lang={lang}, size={len(data)} bytes")
                    strings = parse_rt_string_table(data)
                    print(f"    Parsed {len(strings)} string IDs")
                    for sid in sorted(strings.keys()):
                        val = strings[sid]
                        # Truncate very long values for display
                        display_val = val if len(val) <= 200 else val[:200] + f"... ({len(val)} chars)"
                        safe_print(f"      ID={sid:5d} (0x{sid:04X}): {display_val}")

        # --- RT_DIALOG extraction ---
        print("\n  --- RT_DIALOG Resources (UI Layouts) ---")
        for type_id in [RT_DIALOG, "RT_DIALOG"]:
            if type_id in type_groups:
                for name, lang, data in type_groups[type_id]:
                    block_name = f"name={name}" if name else "unnamed"
                    print(f"\n  [RT_DIALOG] {block_name}, lang={lang}, size={len(data)} bytes")
                    dialog = parse_rt_dialog(data)
                    if "error" in dialog:
                        print(f"    Parse error: {dialog['error']}")
                    print(f"    Controls: {dialog['num_controls'] if 'num_controls' in dialog else 'N/A'}")
                    if dialog.get("font"):
                        print(f"    Font: {dialog['font']}")
                    for ctrl in dialog.get("controls", []):
                        ctrl_class = ctrl.get("class", "")
                        ctrl_text = ctrl.get("text", "")
                        ctrl_id = ctrl.get("id", 0)
                        if ctrl_text:
                            safe_print(f"      Control 0x{ctrl_id:04X}: class={ctrl_class}, text=\"{ctrl_text}\"")
                    for s in dialog.get("strings", []):
                        if s and s not in [ctrl.get("text", "") for ctrl in dialog.get("controls", [])]:
                            safe_print(f"      String: \"{s}\"")

        # --- RT_MENU extraction ---
        print("\n  --- RT_MENU Resources ---")
        for type_id in [RT_MENU, "RT_MENU"]:
            if type_id in type_groups:
                for name, lang, data in type_groups[type_id]:
                    block_name = f"name={name}" if name else "unnamed"
                    print(f"\n  [RT_MENU] {block_name}, lang={lang}, size={len(data)} bytes")
                    menu = parse_rt_menu(data)
                    for item in menu.get("items", []):
                        safe_print(f"      Flags=0x{item['flags']:04X}: \"{item['text']}\"")

        # --- RT_ACCELERATOR extraction ---
        print("\n  --- RT_ACCELERATOR Resources ---")
        for type_id in [RT_ACCELERATOR, "RT_ACCELERATOR"]:
            if type_id in type_groups:
                for name, lang, data in type_groups[type_id]:
                    block_name = f"name={name}" if name else "unnamed"
                    print(f"\n  [RT_ACCELERATOR] {block_name}, lang={lang}, size={len(data)} bytes")
                    acc = parse_rt_accelerator(data)
                    for entry in acc.get("entries", []):
                        print(f"      Key=0x{entry['key']:04X}, ID=0x{entry['id']:04X}, Flags=0x{entry['flags']:04X}")

        # --- RT_VERSION extraction ---
        print("\n  --- RT_VERSION Resources ---")
        for type_id in [RT_VERSION, "RT_VERSION"]:
            if type_id in type_groups:
                for name, lang, data in type_groups[type_id]:
                    block_name = f"name={name}" if name else "unnamed"
                    print(f"\n  [RT_VERSION] {block_name}, lang={lang}, size={len(data)} bytes")
                    ver = parse_rt_version(data)
                    if ver.get("version_info"):
                        vi = ver["version_info"]
                        print(f"    File Version:    {vi.get('file_version', 'N/A')}")
                        print(f"    Product Version: {vi.get('product_version', 'N/A')}")
                        print(f"    File OS:         0x{vi.get('file_os', 0):08X}")
                        print(f"    File Type:       0x{vi.get('file_type', 0):08X}")
                    for sfi in ver.get("string_file_info", []):
                        print(f"    StringFileInfo [{sfi['lang']}]:")
                        for k, v in sfi.get("strings", {}).items():
                            if v:
                                print(f"      {k}: {v}")
                    vfi = ver.get("var_file_info", {})
                    if vfi:
                        print(f"    VarFileInfo:")
                        for k, v in vfi.items():
                            print(f"      {k}: {v}")
                    if ver.get("error"):
                        print(f"    Parse error: {ver['error']}")

        # --- RT_RCDATA / custom resources (DirectUI, HLSL) ---
        print("\n  --- RT_RCDATA and Custom Resources (DirectUI, HLSL, etc.) ---")
        for type_id in [RT_RCDATA, "RT_RCDATA"]:
            if type_id in type_groups:
                for name, lang, data in type_groups[type_id]:
                    name_str = str(name) if name else "unnamed"
                    print(f"\n  [RT_RCDATA] name={name_str}, lang={lang}, size={len(data)} bytes")

                    # Check if name matches known custom types
                    is_custom = False
                    if isinstance(name, str):
                        for pattern, desc in KNOWN_CUSTOM_RCDATA_NAMES.items():
                            if pattern.lower() in name.lower():
                                print(f"    Identified as: {desc}")
                                is_custom = True
                                break

                    # Show first 256 bytes hex dump
                    preview = data[:256]
                    print(f"    Hex preview ({len(data)} bytes total):")
                    for i in range(0, len(preview), 16):
                        chunk = preview[i:i + 16]
                        hex_str = ' '.join(f'{b:02X}' for b in chunk)
                        ascii_str = ''.join(chr(b) if 32 <= b < 127 else '.' for b in chunk)
                        print(f"      {i:04X}: {hex_str:<48s} {ascii_str}")

                    # Try to extract any strings from RT_RCDATA
                    ascii_strs = extract_ascii_strings(data, 6)
                    if ascii_strs:
                        print(f"    Embedded ASCII strings ({len(ascii_strs)}):")
                        for s in ascii_strs[:50]:
                            print(f"      \"{s}\"")
                        if len(ascii_strs) > 50:
                            print(f"      ... and {len(ascii_strs) - 50} more")

        # --- Other resource types ---
        other_types = [RT_BITMAP, RT_ICON, RT_GROUP_ICON, RT_GROUP_CURSOR,
                       RT_CURSOR, RT_FONT, RT_FONTDIR, RT_MANIFEST, RT_HTML,
                       RT_ANICURSOR, RT_ANIICON, RT_VXD, RT_PLUGPLAY, RT_MESSAGETABLE]
        for type_id in other_types:
            if type_id in type_groups:
                type_name = RT_NAMES.get(type_id, str(type_id))
                items = type_groups[type_id]
                print(f"\n  --- {type_name} ({len(items)} resource(s)) ---")
                for name, lang, data in items[:10]:
                    name_str = str(name) if name else "unnamed"
                    print(f"    {name_str}: {len(data)} bytes")
                    # For manifests and HTML, try to show text
                    if type_id in (RT_MANIFEST, RT_HTML):
                        try:
                            text = data.decode("utf-8", errors="replace")[:500]
                            safe_print(f"      Content: {text}")
                        except Exception:
                            pass
                if len(items) > 10:
                    print(f"    ... and {len(items) - 10} more")

    # =====================================================================
    # PART 2: STRING EXTRACTION FROM SECTIONS
    # =====================================================================
    print("\n" + "-" * 80)
    print("  PART 2: STRING EXTRACTION FROM CODE/DATA SECTIONS")
    print("-" * 80)

    # Get section data
    text_data = get_section_data(pe, '.text')
    data_data = get_section_data(pe, '.data')
    rdata_data = get_section_data(pe, '.rdata')
    all_data = text_data + data_data + rdata_data

    print(f"\n  .text size: {len(text_data):,} bytes")
    print(f"  .data size: {len(data_data):,} bytes")
    print(f"  .rdata size: {len(rdata_data):,} bytes")
    print(f"  Total analyzed: {len(all_data):,} bytes")

    # --- ASCII strings ---
    print("\n  --- ASCII Strings ---")
    ascii_strings = extract_ascii_strings(all_data)
    print(f"  Total ASCII strings (>= {MIN_STRING_LEN} chars): {len(ascii_strings)}")

    # Deduplicate while preserving order
    seen = set()
    unique_ascii = []
    for s in ascii_strings:
        if s not in seen:
            seen.add(s)
            unique_ascii.append(s)
    ascii_strings = unique_ascii
    print(f"  Unique ASCII strings: {len(ascii_strings)}")

    # --- UTF-16LE strings ---
    print("\n  --- UTF-16LE Strings ---")
    utf16_strings = extract_utf16le_strings(all_data)
    print(f"  Total UTF-16LE strings (>= {MIN_STRING_LEN} chars): {len(utf16_strings)}")

    seen = set()
    unique_utf16 = []
    for s in utf16_strings:
        if s not in seen:
            seen.add(s)
            unique_utf16.append(s)
    utf16_strings = unique_utf16
    print(f"  Unique UTF-16LE strings: {len(utf16_strings)}")

    # --- Categorize all strings ---
    print("\n  --- String Categories ---")
    all_strings = list(ascii_strings)
    for s in utf16_strings:
        if s not in seen:
            all_strings.append(s)

    categories = categorize_strings(ascii_strings)
    utf16_categories = categorize_strings(utf16_strings)

    for cat_name in ["file_extensions", "registry_paths", "com_clsid_iid",
                      "error_messages", "api_function_names", "ui_labels",
                      "urls", "file_paths"]:
        ascii_count = len(categories.get(cat_name, []))
        utf16_count = len(utf16_categories.get(cat_name, []))
        total = ascii_count + utf16_count
        if total > 0:
            print(f"\n  [{cat_name.upper().replace('_', ' ')}] ({total} strings)")
            # Show all unique from both ASCII and UTF-16
            combined = []
            for s in categories.get(cat_name, []):
                combined.append(f"  ASCII: \"{s}\"")
            for s in utf16_categories.get(cat_name, []):
                if f'  UTF16: "{s}"' not in combined:
                    combined.append(f'  UTF16: "{s}"')
            for item in combined[:100]:
                safe_print(f"    {item}")
            if len(combined) > 100:
                print(f"    ... and {len(combined) - 100} more")
        else:
            print(f"\n  [{cat_name.upper().replace('_', ' ')}] (0 strings)")

    # --- DUMP ALL ASCII STRINGS ---
    print("\n  --- ALL UNIQUE ASCII STRINGS ---")
    for i, s in enumerate(ascii_strings):
        safe_print(f"  [{i:5d}] \"{s}\"")

    # --- DUMP ALL UTF-16LE STRINGS ---
    print("\n  --- ALL UNIQUE UTF-16LE STRINGS ---")
    for i, s in enumerate(utf16_strings):
        safe_print(f"  [{i:5d}] \"{s}\"")

    pe.close()
    print(f"\n  Done analyzing {basename}.")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main():
    print("=" * 100)
    print("  WINDOWS MOVIE MAKER - COMPLETE STRING & RESOURCE EXTRACTION")
    print("=" * 100)

    for filepath in BINARIES:
        if not os.path.exists(filepath):
            print(f"\n  [!] FILE NOT FOUND: {filepath}")
            continue
        analyze_binary(filepath)

    print("\n" + "=" * 100)
    print("  EXTRACTION COMPLETE")
    print("=" * 100)


if __name__ == "__main__":
    main()
