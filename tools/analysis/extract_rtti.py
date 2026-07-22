import re
import struct
from collections import defaultdict

DLL_PATH = r"C:\Users\mcmco\Desktop\WMMR\undecomp\Photo Gallery\MovieMakerCore.dll"

with open(DLL_PATH, "rb") as f:
    data = f.read()

print(f"Loaded {len(data):,} bytes")
print("=" * 120)

# ---------------------------------------------------------------------------
# Parse PE headers to find sections
# ---------------------------------------------------------------------------
e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
num_sections = struct.unpack_from("<H", data, e_lfanew + 6)[0]
opt_hdr_size = struct.unpack_from("<H", data, e_lfanew + 20)[0]
section_start = e_lfanew + 24 + opt_hdr_size

sections = []
for i in range(num_sections):
    off = section_start + i * 40
    name = data[off:off+8].rstrip(b'\x00').decode('ascii', errors='replace')
    vsize = struct.unpack_from("<I", data, off + 8)[0]
    vaddr = struct.unpack_from("<I", data, off + 12)[0]
    rawsize = struct.unpack_from("<I", data, off + 16)[0]
    rawptr = struct.unpack_from("<I", data, off + 20)[0]
    sections.append((name, vaddr, vsize, rawptr, rawsize))
    print(f"Section: {name:8s}  VA=0x{vaddr:08X}  VSize=0x{vsize:08X}  Raw=0x{rawptr:08X}  RawSize=0x{rawsize:08X}")

print("=" * 120)

# ---------------------------------------------------------------------------
# Extract MSVC RTTI type names
# ---------------------------------------------------------------------------
# Comprehensive patterns covering all MSVC RTTI name forms
patterns = [
    re.compile(rb'\?A[VUWE][A-Za-z0-9_@$]+@@'),          # Standard: ?AVClassName@@
    re.compile(rb'\?\$A[VUWE][A-Za-z0-9_@$]+@@'),         # Template: ?$AVClassName@@
    re.compile(rb'\?A\?A[VUWE][A-Za-z0-9_@$]+@@'),        # Nested:   ?A?AVClassName@@
    re.compile(rb'\?A\?\$A[VUWE][A-Za-z0-9_@$]+@@'),      # Nested template
    re.compile(rb'\?R[A-Za-z0-9_@$]+@@'),                  # Reference types
    re.compile(rb'\?P[A-Za-z0-9_@$]+@@'),                  # Pointer types
    re.compile(rb'\?PC[A-Za-z0-9_@$]+@@'),                 # Pointer to const
]

rtti_matches = []
seen = set()
for pat in patterns:
    for m in pat.finditer(data):
        s = m.group().decode('ascii', errors='replace')
        if s not in seen:
            seen.add(s)
            rtti_matches.append(s)

rtti_matches.sort()
print(f"\nFound {len(rtti_matches)} unique RTTI type name strings")

# ---------------------------------------------------------------------------
# Parse RTTI names
# ---------------------------------------------------------------------------
def parse_rtti_name(raw):
    s = raw
    while s.startswith("?A"):
        s = s[2:]
    if s.endswith("@@"):
        s = s[:-2]
    parts = s.split("@")
    type_tag = parts[0][0] if parts[0] else '?'
    type_names = {'V': 'class', 'U': 'struct', 'W': 'union', 'E': 'enum'}
    type_str = type_names.get(type_tag, 'unknown')
    class_name = parts[0][1:] if len(parts[0]) > 1 else parts[0]
    namespaces = parts[1:] if len(parts) > 1 else []
    return type_str, namespaces, class_name

parsed = []
for r in rtti_matches:
    t, ns, cn = parse_rtti_name(r)
    parsed.append((t, ns, cn, r))

# ---------------------------------------------------------------------------
# Group by functional area
# ---------------------------------------------------------------------------
groups = defaultdict(list)
for t, ns, cn, raw in parsed:
    full_name = "::".join(ns + [cn]) if ns else cn
    lower = full_name.lower()
    if "hmrengine" in lower:
        groups["HMREngine* (Rendering Engine)"].append((t, ns, cn, raw))
    elif "hmrasource" in lower or "avsource" in lower:
        groups["HMRAVSource* (A/V Sources)"].append((t, ns, cn, raw))
    elif "storyboard" in lower:
        groups["StoryboardManager* (Timeline)"].append((t, ns, cn, raw))
    elif "sundance" in lower:
        groups["SundanceApp* (Main App)"].append((t, ns, cn, raw))
    elif lower.startswith("d3d") or "direct3d" in lower:
        groups["D3D* (DirectX)"].append((t, ns, cn, raw))
    elif re.match(r'^I[A-Z]', cn) and len(cn) > 1 and cn[1].isupper():
        groups["I* Interfaces (COM)"].append((t, ns, cn, raw))
    elif cn.startswith("C") and len(cn) > 1 and cn[1].isupper():
        groups["C* Classes (General)"].append((t, ns, cn, raw))
    else:
        groups["Other"].append((t, ns, cn, raw))

for gname in sorted(groups.keys()):
    items = groups[gname]
    print(f"\n### {gname} ({len(items)} classes)")
    print("-" * 120)
    for t, ns, cn, raw in sorted(items, key=lambda x: "::".join(x[1] + [x[2]]).lower()):
        ns_str = "::".join(ns) + "::" if ns else ""
        print(f"  {t:7s}  {ns_str}{cn:50s}  [{raw}]")

# ---------------------------------------------------------------------------
# Complete sorted RTTI list
# ---------------------------------------------------------------------------
print("\n" + "=" * 120)
print("COMPLETE RTTI CLASS LIST (sorted alphabetically)")
print("=" * 120)
for t, ns, cn, raw in sorted(parsed, key=lambda x: "::".join(x[1] + [x[2]]).lower()):
    ns_str = "::".join(ns) + "::" if ns else ""
    print(f"  {t:7s}  {ns_str}{cn:50s}  [{raw}]")

# ---------------------------------------------------------------------------
# .rsrc section - extract meaningful strings (filter out image data noise)
# ---------------------------------------------------------------------------
print("\n" + "=" * 120)
print(".rsrc SECTION - MEANINGFUL STRINGS")
print("=" * 120)

rsrc = None
for name, va, vs, rp, rs in sections:
    if name == ".rsrc":
        rsrc = (rp, rs, va)
        break

if rsrc:
    rp, rs, va = rsrc
    rsrc_data = data[rp:rp+rs]
    print(f".rsrc at file offset 0x{rp:X}, size {rs:,} bytes")

    # Extract readable ASCII strings, filter to meaningful ones
    ascii_strings = []
    current = []
    for i in range(len(rsrc_data)):
        b = rsrc_data[i]
        if 32 <= b < 127:
            current.append(chr(b))
        else:
            if len(current) >= 8:
                s = "".join(current)
                # Filter out obvious binary noise
                if not all(c in set('0123456789abcdefABCDEF') for c in s):
                    ascii_strings.append(s)
            current = []
    if len(current) >= 8:
        ascii_strings.append("".join(current))

    # Deduplicate and sort
    ascii_strings = sorted(set(ascii_strings))
    print(f"\nMeaningful ASCII strings ({len(ascii_strings)}):")
    for s in ascii_strings:
        print(f"  {s}")

    # Unicode strings
    uni_strings = []
    current = []
    i = 0
    while i < len(rsrc_data) - 1:
        lo = rsrc_data[i]
        hi = rsrc_data[i+1]
        if hi == 0 and 32 <= lo < 127:
            current.append(chr(lo))
            i += 2
        else:
            if len(current) >= 6:
                s = "".join(current)
                if not all(c in set('0123456789abcdefABCDEF') for c in s):
                    uni_strings.append(s)
            current = []
            i += 2
    if len(current) >= 6:
        uni_strings.append("".join(current))

    uni_strings = sorted(set(uni_strings))
    print(f"\nUnicode strings ({len(uni_strings)}):")
    for s in uni_strings:
        print(f"  {s}")

    # Parse resource directory tree for string tables (type 3 = RT_STRING)
    # String tables: ID >> 4 = table index, ID & 0xF = string index
    # But easier: just scan for the known WLX/XAML element names which are the real content
    print("\n--- XAML/WLX UI Element Names (filtered) ---")
    ui_strings = sorted(set(s for s in ascii_strings if any(kw in s for kw in [
        'ide', 'Property', 'Behavior', 'Layout', 'Dialog', 'Button', 'Timeline',
        'Preview', 'Track', 'Slider', 'Menu', 'Pane', 'Text', 'Sundance',
        'Homer', 'CommandBar', 'Progress', 'ScrollBar', 'Popup', 'WLX',
        'Storyboard', 'Audio', 'Video', 'Trim', 'Effect', 'Webcam',
        'Narration', 'Publish', 'Options', 'Import', 'Title', 'Caption',
        'Transition', 'Animation', 'KeyFrame', 'Render', 'Export'
    ])))
    for s in ui_strings:
        print(f"  {s}")
else:
    print("No .rsrc section found!")

# ---------------------------------------------------------------------------
# CLSID/IID GUIDs
# ---------------------------------------------------------------------------
print("\n" + "=" * 120)
print("CLSID / IID / GUID REFERENCES")
print("=" * 120)

# String-form GUIDs
guid_str_pattern = re.compile(rb'[{(]?[0-9a-fA-F]{8}[-]?[0-9a-fA-F]{4}[-]?[0-9a-fA-F]{4}[-]?[0-9a-fA-F]{4}[-]?[0-9a-fA-F]{12}[})]?')
guid_strs = sorted(set(m.group().decode('ascii', errors='replace') for m in guid_str_pattern.finditer(data)))
print(f"\nString-form GUIDs ({len(guid_strs)}):")
for g in guid_strs:
    print(f"  {g}")

# Binary GUIDs with proper version/variant
binary_guids = []
seen_guids = set()
for i in range(len(data) - 16):
    b = data[i:i+16]
    if b == b'\x00' * 16:
        continue
    w2 = struct.unpack_from("<H", b, 6)[0]
    ver = (w2 >> 12) & 0xF
    if 1 <= ver <= 5:
        var_byte = b[8]
        if (var_byte & 0xC0) == 0x80:
            d1 = struct.unpack_from("<I", b, 0)[0]
            w1 = struct.unpack_from("<H", b, 4)[0]
            guid_str = f"{{{d1:08X}-{w1:04X}-{w2:04X}-{b[8]:02X}{b[9]:02X}-{b[10]:02X}{b[11]:02X}{b[12]:02X}{b[13]:02X}{b[14]:02X}{b[15]:02X}}}"
            if guid_str not in seen_guids:
                seen_guids.add(guid_str)
                binary_guids.append((i, guid_str))

print(f"\nBinary GUIDs (RFC 4122 variant) ({len(binary_guids)}):")
for off, g in sorted(binary_guids, key=lambda x: x[0]):
    print(f"  offset=0x{off:08X}  {g}")

# COM-related string references
com_refs = set()
for m in re.finditer(rb'(CLSID|IID|LibID|TypeLib|ProgID|CATID)[_\s]*[=:{]\s*[{(]?([0-9a-fA-F\-\{\}]+)', data, re.IGNORECASE):
    com_refs.add(m.group().decode('ascii', errors='replace'))
if com_refs:
    print(f"\nCOM registration references ({len(com_refs)}):")
    for r in sorted(com_refs):
        print(f"  {r}")

# ---------------------------------------------------------------------------
# Vtable patterns (optimized: scan .rdata in 4-byte steps)
# ---------------------------------------------------------------------------
print("\n" + "=" * 120)
print("POTENTIAL COM VTABLE PATTERNS")
print("=" * 120)

text = None
data_sec = None
for name2, va2, vs2, rp2, rs2 in sections:
    if name2 == ".text":
        text = (va2, vs2)
    elif name2 == ".data":
        data_sec = (rp2, rs2, va2)

# No .rdata exists in this binary (merged into .text). Scan .data for vtable pointers.
potential_vtables = []
if text:
    text_va, text_vs = text
    text_end = text_va + text_vs
    if data_sec:
        rp_d, rs_d, va_d = data_sec
        scan_data = data[rp_d:rp_d+rs_d]
        i = 0
        while i < len(scan_data) - 20:
            count = 0
            j = i
            while j < len(scan_data) - 4:
                ptr = struct.unpack_from("<I", scan_data, j)[0]
                if text_va <= ptr < text_end:
                    count += 1
                    j += 4
                else:
                    break
            if count >= 5:
                file_off = rp_d + i
                rva = va_d + i
                ctx = data[max(0, file_off - 128):file_off]
                rtti_near = re.findall(rb'\?A[VUWE]\w+@@', ctx)
                hint = rtti_near[-1].decode('ascii', errors='replace') if rtti_near else "???"
                potential_vtables.append((file_off, rva, count, hint))
                i = j
            else:
                i += 4
    else:
        print("  No .data section found")

    # Also scan the read-only portions of .text for vtable-like arrays
    # (read-only data is often appended after code in merged sections)
    # Look for consecutive pointers within .text VA range
    text_rp = None
    for name2, va2, vs2, rp2, rs2 in sections:
        if name2 == ".text":
            text_rp = rp2
            text_rawsz = rs2
            break
    if text_rp:
        # Scan last 20% of .text raw data (likely contains read-only data)
        scan_start = int(text_rawsz * 0.8)
        scan_data = data[text_rp + scan_start:text_rp + text_rawsz]
        i = 0
        while i < len(scan_data) - 20:
            count = 0
            j = i
            while j < len(scan_data) - 4:
                ptr = struct.unpack_from("<I", scan_data, j)[0]
                if text_va <= ptr < text_end:
                    count += 1
                    j += 4
                else:
                    break
            if count >= 6:
                file_off = text_rp + scan_start + i
                rva_offset = scan_start + i
                rva = text_va + rva_offset
                # Check for RTTI locator just before this vtable (-16 bytes is RTTIClassHierarchyDescriptor, -8 is RTTICompleteObjectLocator)
                ctx = data[max(0, file_off - 256):file_off]
                rtti_near = re.findall(rb'\?A[VUWE]\w+@@', ctx)
                hint = rtti_near[-1].decode('ascii', errors='replace') if rtti_near else "???"
                # Avoid duplicates (same file offset)
                if not potential_vtables or potential_vtables[-1][0] != file_off:
                    potential_vtables.append((file_off, rva, count, hint))
                i = j
            else:
                i += 4

    print(f"\nFound {len(potential_vtables)} potential vtables (5+ consecutive code pointers):")
    for foff, rva, cnt, hint in potential_vtables[:150]:
        print(f"  vtable @ file=0x{foff:08X}  RVA=0x{rva:08X}  entries={cnt:3d}  near: {hint}")
    if len(potential_vtables) > 150:
        print(f"  ... and {len(potential_vtables) - 150} more")
else:
    print("  Could not locate .text section")

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
print("\n" + "=" * 120)
print("SUMMARY")
print("=" * 120)
class_count = sum(1 for t,_,_,_ in parsed if t=='class')
struct_count = sum(1 for t,_,_,_ in parsed if t=='struct')
union_count = sum(1 for t,_,_,_ in parsed if t=='union')
enum_count = sum(1 for t,_,_,_ in parsed if t=='enum')
print(f"Total unique RTTI type names: {len(rtti_matches)}")
print(f"  Classes: {class_count}")
print(f"  Structs: {struct_count}")
print(f"  Unions:  {union_count}")
print(f"  Enums:   {enum_count}")
print(f"String-form GUIDs: {len(guid_strs)}")
print(f"Binary GUIDs (RFC 4122): {len(binary_guids)}")
print(f"Potential vtables: {len(potential_vtables)}")

all_ns = set()
for t, ns, cn, raw in parsed:
    if ns:
        all_ns.add("::".join(ns))
print(f"\nUnique namespaces ({len(all_ns)}):")
for ns in sorted(all_ns):
    count = sum(1 for _,n,_,_ in parsed if "::".join(n) == ns)
    print(f"  {ns:60s}  ({count} classes)")
