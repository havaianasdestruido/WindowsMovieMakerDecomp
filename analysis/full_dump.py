import pefile
import struct
import os
import sys

PHOTO_GALLERY = r"C:\Users\mcmco\Desktop\WMMR\undecomp\Photo Gallery"
OUTPUT_FILE = r"C:\Users\mcmco\Desktop\WMMR\analysis\full_dump_output.txt"

DLLS = {
    "MovieMakerCore.dll": os.path.join(PHOTO_GALLERY, "MovieMakerCore.dll"),
    "MovieMaker.exe": os.path.join(PHOTO_GALLERY, "MovieMaker.exe"),
    "WLXPhotoBase.dll": os.path.join(PHOTO_GALLERY, "WLXPhotoBase.dll"),
}


class TeeWriter:
    def __init__(self, *writers):
        self.writers = writers

    def write(self, s):
        for w in self.writers:
            w.write(s)
            w.flush()

    def flush(self):
        for w in self.writers:
            w.flush()


def get_resource_entries(pe):
    if not hasattr(pe, "DIRECTORY_ENTRY_RESOURCE"):
        return []
    return pe.DIRECTORY_ENTRY_RESOURCE.entries


def iter_resource_type(pe, type_id):
    for res_type in get_resource_entries(pe):
        if res_type.struct.Id == type_id:
            if res_type.directory and res_type.directory.entries:
                for res_id in res_type.directory.entries:
                    if res_id.directory and res_id.directory.entries:
                        for res_lang in res_id.directory.entries:
                            yield res_id, res_lang


def dump_exports(pe, name):
    out = []
    out.append(f"{'='*80}")
    out.append(f"EXPORTS FOR {name}")
    out.append(f"{'='*80}")
    if not hasattr(pe, "DIRECTORY_ENTRY_EXPORT"):
        out.append("  (no exports)")
        return "\n".join(out)
    exp = pe.DIRECTORY_ENTRY_EXPORT
    dll_name = exp.name.decode() if exp.name else "<unknown>"
    out.append(f"  DLL name: {dll_name}")
    out.append(f"  Ordinal base: {exp.struct.Base}")
    out.append(f"  Number of exports: {len(exp.symbols)}")
    out.append("")
    for s in exp.symbols:
        if s.name:
            out.append(f"  Ordinal {s.ordinal:5d}  0x{s.address:08X}  {s.name.decode()}")
        else:
            out.append(f"  Ordinal {s.ordinal:5d}  0x{s.address:08X}  (by ordinal only)")
    return "\n".join(out)


def dump_imports(pe, name):
    out = []
    out.append(f"{'='*80}")
    out.append(f"IMPORTS FOR {name}")
    out.append(f"{'='*80}")
    if not hasattr(pe, "DIRECTORY_ENTRY_IMPORT"):
        out.append("  (no imports)")
        return "\n".join(out)
    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        dll_name = entry.dll.decode() if entry.dll else "<unknown>"
        out.append(f"\n  --- {dll_name} ---")
        for imp in entry.imports:
            if imp.name:
                out.append(f"    {imp.name.decode()}")
            else:
                out.append(f"    ord:{imp.ordinal}")
    return "\n".join(out)


def dump_delay_imports(pe, name):
    out = []
    out.append(f"{'='*80}")
    out.append(f"DELAY-LOAD IMPORTS FOR {name}")
    out.append(f"{'='*80}")
    if not hasattr(pe, "DIRECTORY_ENTRY_DELAY_IMPORT"):
        out.append("  (no delay-load imports)")
        return "\n".join(out)
    for entry in pe.DIRECTORY_ENTRY_DELAY_IMPORT:
        dll_name = entry.dll.decode() if entry.dll else "<unknown>"
        out.append(f"\n  --- {dll_name} ---")
        for imp in entry.imports:
            if imp.name:
                out.append(f"    {imp.name.decode()}")
            else:
                out.append(f"    ord:{imp.ordinal}")
    return "\n".join(out)


def dump_rtti(pe, name):
    out = []
    out.append(f"{'='*80}")
    out.append(f"RTTI TYPE NAMES FOR {name}")
    out.append(f"{'='*80}")
    pattern = b"\x3f\x41\x56"
    found = []
    found_set = set()
    for section in pe.sections:
        data = section.get_data()
        offset = 0
        while True:
            idx = data.find(pattern, offset)
            if idx == -1:
                break
            start = idx
            while start > 0 and data[start - 1] != 0:
                start -= 1
            end = idx
            while end < len(data) and data[end] != 0:
                end += 1
            name_bytes = data[start:end]
            try:
                name_str = name_bytes.decode("ascii")
            except Exception:
                name_str = repr(name_bytes)
            if name_str and name_str not in found_set:
                found_set.add(name_str)
                rva_start = section.VirtualAddress + start
                found.append((rva_start, name_str))
            offset = idx + 3
    found.sort(key=lambda x: x[0])
    for rva, ns in found:
        out.append(f"  RVA 0x{rva:08X}  {ns}")
    out.append(f"\n  Total unique RTTI names: {len(found)}")
    return "\n".join(out)


def dump_rt_string_resources(pe, name):
    out = []
    out.append(f"{'='*80}")
    out.append(f"RT_STRING RESOURCES FOR {name}")
    out.append(f"{'='*80}")
    found = False
    for res_id, res_lang in iter_resource_type(pe, 6):
        found = True
        table_id = res_id.struct.Id
        data_rva = res_lang.data.struct.OffsetToData
        size = res_lang.data.struct.Size
        data = pe.get_memory_mapped_image()[data_rva:data_rva + size]
        table_base = table_id * 16
        out.append(f"  String table ID: {table_id} (strings {table_base}-{table_base+15})")
        offset = 0
        for i in range(16):
            if offset + 2 > len(data):
                break
            str_len = struct.unpack_from("<H", data, offset)[0]
            offset += 2
            if str_len > 0 and offset + str_len * 2 <= len(data):
                raw = data[offset:offset + str_len * 2]
                try:
                    s = raw.decode("utf-16-le")
                except Exception:
                    s = repr(raw)
                out.append(f"      [{table_base + i:5d}] {s}")
                offset += str_len * 2
    if not found:
        out.append("  (no RT_STRING resources)")
    return "\n".join(out)


def dump_guids(pe, name):
    out = []
    out.append(f"{'='*80}")
    out.append(f"CLSID/IID GUIDS FOR {name}")
    out.append(f"{'='*80}")
    guids = {}
    import re
    guid_re = re.compile(rb"\{[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}\}")
    for section in pe.sections:
        if section.SizeOfRawData == 0:
            continue
        data = section.get_data()
        for m in guid_re.finditer(data):
            s = m.group(0).decode("ascii").upper()
            rva = section.VirtualAddress + m.start()
            if s not in guids:
                guids[s] = rva

    if not guids:
        out.append("  (no GUIDs found)")
    else:
        for guid_str in sorted(guids.keys(), key=lambda g: guids[g]):
            out.append(f"  RVA 0x{guids[guid_str]:08X}  {guid_str}")
    out.append(f"\n  Total unique GUIDs: {len(guids)}")
    return "\n".join(out)


def dump_rt_version(pe, name):
    out = []
    out.append(f"{'='*80}")
    out.append(f"RT_VERSION RESOURCE FOR {name}")
    out.append(f"{'='*80}")
    found = False
    for res_id, res_lang in iter_resource_type(pe, 16):
        found = True
        data_rva = res_lang.data.struct.OffsetToData
        size = res_lang.data.struct.Size
        data = pe.get_memory_mapped_image()[data_rva:data_rva + size]
        if len(data) < 52:
            out.append(f"  Version resource too small ({len(data)} bytes)")
            continue
        length, value_len, w_type = struct.unpack_from("<HHH", data, 0)
        out.append(f"  Total size: {length} bytes")
        out.append(f"  Header type: {w_type}")

        key_offset = 6
        end_null = data.find(b"\x00\x00", key_offset)
        if end_null == -1:
            end_null = key_offset + 34
        try:
            key = data[key_offset:end_null + 1].decode("utf-16-le")
        except Exception:
            key = data[key_offset:key_offset + 34].decode("utf-16-le", errors="replace")
        out.append(f"  Key: {key}")

        val_offset = end_null + 2
        if val_offset % 4:
            val_offset += 4 - (val_offset % 4)

        if val_offset + 52 <= len(data):
            v_sig = struct.unpack_from("<I", data, val_offset)[0]
            out.append(f"  Signature: 0x{v_sig:08X} {'(valid VS_FIXEDFILEINFO)' if v_sig == 0xFEEF04BD else '(INVALID)'}")
            v_struc_ver = struct.unpack_from("<I", data, val_offset + 4)[0]
            v_file_ver_ms = struct.unpack_from("<I", data, val_offset + 8)[0]
            v_file_ver_ls = struct.unpack_from("<I", data, val_offset + 12)[0]
            v_prod_ver_ms = struct.unpack_from("<I", data, val_offset + 16)[0]
            v_prod_ver_ls = struct.unpack_from("<I", data, val_offset + 20)[0]
            v_file_flags_mask = struct.unpack_from("<I", data, val_offset + 24)[0]
            v_file_flags = struct.unpack_from("<I", data, val_offset + 28)[0]
            v_file_os = struct.unpack_from("<I", data, val_offset + 32)[0]
            v_file_type = struct.unpack_from("<I", data, val_offset + 36)[0]
            v_file_subtype = struct.unpack_from("<I", data, val_offset + 40)[0]
            v_file_date_ms = struct.unpack_from("<I", data, val_offset + 44)[0]
            v_file_date_ls = struct.unpack_from("<I", data, val_offset + 48)[0]
            fv = f"{v_file_ver_ms >> 16}.{v_file_ver_ms & 0xFFFF}.{v_file_ver_ls >> 16}.{v_file_ver_ls & 0xFFFF}"
            pv = f"{v_prod_ver_ms >> 16}.{v_prod_ver_ms & 0xFFFF}.{v_prod_ver_ls >> 16}.{v_prod_ver_ls & 0xFFFF}"
            out.append(f"  Struct version: {v_struc_ver >> 16}.{v_struc_ver & 0xFFFF}")
            out.append(f"  File version: {fv}")
            out.append(f"  Product version: {pv}")
            out.append(f"  Flags mask: 0x{v_file_flags_mask:08X}")
            out.append(f"  Flags: 0x{v_file_flags:08X}")
            out.append(f"  OS: 0x{v_file_os:08X}")
            out.append(f"  File type: 0x{v_file_type:08X}")
            out.append(f"  File subtype: 0x{v_file_subtype:08X}")
            out.append(f"  File date: {v_file_date_ms:08X}.{v_file_date_ls:08X}")

        child_offset = val_offset + 52
        while child_offset + 8 <= len(data):
            block_len = struct.unpack_from("<H", data, child_offset)[0]
            if block_len == 0 or child_offset + block_len > len(data):
                break
            val_len = struct.unpack_from("<H", data, child_offset + 2)[0]
            w_type_child = struct.unpack_from("<H", data, child_offset + 4)[0]
            child_key_off = child_offset + 6
            end_key = child_key_off
            while end_key + 1 < child_offset + block_len:
                if data[end_key] == 0 and data[end_key + 1] == 0:
                    break
                end_key += 2
            try:
                child_key = data[child_key_off:end_key].decode("utf-16-le")
            except Exception:
                child_key = "<decode error>"
            out.append(f"\n  Block: {child_key} (size={block_len}, val_len={val_len})")

            if w_type_child == 1 and child_key == "StringFileInfo":
                st_offset = end_key + 2
                if st_offset % 4:
                    st_offset += 4 - (st_offset % 4)
                block_end = child_offset + block_len
                while st_offset + 8 <= block_end:
                    st_len = struct.unpack_from("<H", data, st_offset)[0]
                    if st_len == 0 or st_offset + st_len > block_end:
                        break
                    st_key_off = st_offset + 6
                    st_key_end = st_key_off
                    while st_key_end + 1 < st_offset + st_len:
                        if data[st_key_end] == 0 and data[st_key_end + 1] == 0:
                            break
                        st_key_end += 2
                    try:
                        st_key = data[st_key_off:st_key_end].decode("utf-16-le")
                    except Exception:
                        st_key = "<error>"
                    out.append(f"    StringTable: {st_key}")

                    str_offset = st_key_end + 2
                    if str_offset % 4:
                        str_offset += 4 - (str_offset % 4)
                    while str_offset + 8 <= st_offset + st_len:
                        s_len = struct.unpack_from("<H", data, str_offset)[0]
                        if s_len == 0 or str_offset + s_len > st_offset + st_len:
                            break
                        s_val_len = struct.unpack_from("<H", data, str_offset + 2)[0]
                        s_key_off = str_offset + 6
                        s_key_end2 = s_key_off
                        while s_key_end2 + 1 < str_offset + s_len:
                            if data[s_key_end2] == 0 and data[s_key_end2 + 1] == 0:
                                break
                            s_key_end2 += 2
                        try:
                            s_key = data[s_key_off:s_key_end2].decode("utf-16-le")
                        except Exception:
                            s_key = "<error>"
                        val_data_off = s_key_end2 + 2
                        if val_data_off % 4:
                            val_data_off += 4 - (val_data_off % 4)
                        val_end = str_offset + s_len
                        if s_val_len > 0:
                            byte_len = s_val_len * 2
                            raw_val = data[val_data_off:val_data_off + byte_len]
                            try:
                                s_val = raw_val.decode("utf-16-le")
                            except Exception:
                                s_val = repr(raw_val)
                        else:
                            s_val = ""
                        out.append(f"      {s_key} = {s_val}")
                        str_offset = val_end
                        if str_offset % 4:
                            str_offset += 4 - (str_offset % 4)
                    st_offset = str_offset
                    if st_offset % 4:
                        st_offset += 4 - (st_offset % 4)

            elif w_type_child == 1 and child_key == "VarFileInfo":
                var_offset = end_key + 2
                if var_offset % 4:
                    var_offset += 4 - (var_offset % 4)
                block_end = child_offset + block_len
                while var_offset + 8 <= block_end:
                    v_len = struct.unpack_from("<H", data, var_offset)[0]
                    if v_len == 0 or var_offset + v_len > block_end:
                        break
                    v_val_len = struct.unpack_from("<H", data, var_offset + 2)[0]
                    v_key_off = var_offset + 6
                    v_key_end = v_key_off
                    while v_key_end + 1 < var_offset + v_len:
                        if data[v_key_end] == 0 and data[v_key_end + 1] == 0:
                            break
                        v_key_end += 2
                    try:
                        v_key = data[v_key_off:v_key_end].decode("utf-16-le")
                    except Exception:
                        v_key = "<error>"
                    val_d = v_key_end + 2
                    if val_d % 4:
                        val_d += 4 - (val_d % 4)
                    if v_val_len > 0 and val_d + v_val_len * 2 <= block_end:
                        raw = data[val_d:val_d + v_val_len * 2]
                        try:
                            v_val = raw.decode("utf-16-le")
                        except Exception:
                            v_val = repr(raw)
                        out.append(f"    {v_key} = {v_val}")
                    else:
                        out.append(f"    {v_key}")
                    var_offset = var_offset + v_len
                    if var_offset % 4:
                        var_offset += 4 - (var_offset % 4)

            child_offset += block_len
            if child_offset % 4:
                child_offset += 4 - (child_offset % 4)

    if not found:
        out.append("  (no RT_VERSION resources)")
    return "\n".join(out)


def dump_all_resources(pe, name):
    out = []
    out.append(f"{'='*80}")
    out.append(f"ALL RESOURCES FOR {name}")
    out.append(f"{'='*80}")
    if not hasattr(pe, "DIRECTORY_ENTRY_RESOURCE"):
        out.append("  (no resources)")
        return "\n".join(out)

    RT_NAMES = {
        1: "RT_CURSOR", 2: "RT_BITMAP", 3: "RT_ICON", 4: "RT_MENU",
        5: "RT_DIALOG", 6: "RT_STRING", 7: "RT_FONTDIR", 8: "RT_FONT",
        9: "RT_ACCELERATOR", 10: "RT_RCDATA", 11: "RT_MESSAGETABLE",
        12: "RT_GROUP_CURSOR", 14: "RT_GROUP_ICON", 16: "RT_VERSION",
        17: "RT_DLGINCLUDE", 19: "RT_PLUGPLAY", 20: "RT_VXD",
        21: "RT_ANICURSOR", 22: "RT_ANIICON", 23: "RT_HTML",
        24: "RT_MANIFEST",
    }

    for res_type in get_resource_entries(pe):
        type_id = res_type.struct.Id
        type_label = RT_NAMES.get(type_id, f"id={type_id}")

        count = 0
        if res_type.directory and res_type.directory.entries:
            for res_id in res_type.directory.entries:
                if res_id.directory and res_id.directory.entries:
                    count += len(res_id.directory.entries)

        out.append(f"\n  {type_label}: {count} resource(s)")

        if res_type.directory and res_type.directory.entries:
            for res_id in res_type.directory.entries:
                id_val = res_id.struct.Id
                if res_id.directory and res_id.directory.entries:
                    for res_lang in res_id.directory.entries:
                        lang_id = res_lang.struct.Id if res_lang.struct else 0
                        sz = res_lang.data.struct.Size if res_lang.data else 0
                        rva = res_lang.data.struct.OffsetToData if res_lang.data else 0
                        out.append(f"    ID={id_val}  lang_id={lang_id}  size={sz}  RVA=0x{rva:08X}")
    return "\n".join(out)


def dump_manifest(pe, name):
    out = []
    out.append(f"{'='*80}")
    out.append(f"MANIFEST XML FOR {name}")
    out.append(f"{'='*80}")
    found = False
    for res_id, res_lang in iter_resource_type(pe, 24):
        found = True
        data_rva = res_lang.data.struct.OffsetToData
        size = res_lang.data.struct.Size
        data = pe.get_memory_mapped_image()[data_rva:data_rva + size]
        try:
            xml = data.decode("utf-8-sig")
        except Exception:
            xml = data.decode("utf-8", errors="replace")
        out.append(f"  Resource ID: {res_id.struct.Id}")
        lang_id = res_lang.struct.Id if res_lang.struct else 0
        out.append(f"  Language ID: {lang_id}")
        out.append(xml)
    if not found:
        out.append("  (no RT_MANIFEST resources)")
    return "\n".join(out)


def process_dll(label, filepath):
    out = []
    out.append(f"\n{'#'*80}")
    out.append(f"#  PROCESSING: {label}")
    out.append(f"#  FILE: {filepath}")
    out.append(f"{'#'*80}\n")

    pe = pefile.PE(filepath)
    out.append(f"  Machine: 0x{pe.FILE_HEADER.Machine:04X}")
    out.append(f"  Sections: {len(pe.sections)}")
    for s in pe.sections:
        sn = s.Name.decode("utf-8", errors="replace").rstrip("\x00")
        out.append(f"    {sn:8s}  VAddr=0x{s.VirtualAddress:08X}  VSize=0x{s.Misc_VirtualSize:08X}  RawSize=0x{s.SizeOfRawData:08X}  Chars=0x{s.Characteristics:08X}")
    out.append(f"  Subsystem: {pe.OPTIONAL_HEADER.Subsystem}")
    out.append(f"  DLL Characteristics: 0x{pe.OPTIONAL_HEADER.DllCharacteristics:04X}")
    out.append(f"  ImageBase: 0x{pe.OPTIONAL_HEADER.ImageBase:08X}")
    out.append(f"  EntryPoint: 0x{pe.OPTIONAL_HEADER.AddressOfEntryPoint:08X}")
    out.append("")

    out.append(dump_exports(pe, label))
    out.append("")
    out.append(dump_imports(pe, label))
    out.append("")
    out.append(dump_delay_imports(pe, label))
    out.append("")
    out.append(dump_rtti(pe, label))
    out.append("")
    out.append(dump_rt_string_resources(pe, label))
    out.append("")
    out.append(dump_guids(pe, label))
    out.append("")
    out.append(dump_rt_version(pe, label))
    out.append("")
    out.append(dump_all_resources(pe, label))
    out.append("")
    out.append(dump_manifest(pe, label))
    out.append("")

    pe.close()
    return "\n".join(out)


def main():
    outf = open(OUTPUT_FILE, "w", encoding="utf-8")
    tee = TeeWriter(sys.stdout, outf)

    tee.write("="*80 + "\n")
    tee.write("FULL BINARY DUMP FOR SOURCE RECONSTRUCTION\n")
    tee.write(f"Date: {__import__('datetime').datetime.now().isoformat()}\n")
    tee.write("="*80 + "\n\n")

    for label, path in DLLS.items():
        if not os.path.isfile(path):
            tee.write(f"\n*** FILE NOT FOUND: {path} ***\n")
            continue
        result = process_dll(label, path)
        tee.write(result + "\n\n")

    tee.write("="*80 + "\n")
    tee.write("END OF DUMP\n")
    tee.write("="*80 + "\n")

    outf.close()
    print(f"\nOutput written to: {OUTPUT_FILE}")


if __name__ == "__main__":
    main()
