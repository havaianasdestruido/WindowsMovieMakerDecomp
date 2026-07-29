#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdio>
#include <string>
#include <vector>
#include <map>

// -----------------------------------------------------------------------
// Resource type name lookup
// -----------------------------------------------------------------------
static const struct { WORD id; const wchar_t* name; } RT_NAMES[] = {
    {1, L"CURSOR"}, {2, L"BITMAP"}, {3, L"ICON"}, {4, L"MENU"},
    {5, L"DIALOG"}, {6, L"STRING"}, {7, L"FONTDIR"}, {8, L"FONT"},
    {9, L"ACCELERATOR"}, {10, L"RCDATA"}, {11, L"MESSAGETABLE"},
    {12, L"GROUP_CURSOR"}, {14, L"GROUP_ICON"}, {16, L"VERSION"},
    {17, L"DLGINCLUDE"}, {19, L"PLUGPLAY"}, {20, L"VXD"},
    {21, L"ANICURSOR"}, {22, L"ANIICON"}, {23, L"HTML"},
    {24, L"MANIFEST"},
};

static const wchar_t* GetTypeName(WORD typeId) {
    static wchar_t buf[32];
    for (auto& t : RT_NAMES)
        if (t.id == typeId) return t.name;
    wsprintfW(buf, L"TYPE_%u", typeId);
    return buf;
}

// -----------------------------------------------------------------------
// File I/O helpers
// -----------------------------------------------------------------------
static bool WriteBin(const std::wstring& path, const void* data, DWORD size) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return false;
    DWORD written = 0;
    WriteFile(h, data, size, &written, NULL);
    CloseHandle(h);
    return written == size;
}

static bool WriteUtf8(const std::wstring& path, const std::wstring& text) {
    std::string utf8;
    int len = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(),
                                  NULL, 0, NULL, NULL);
    if (len <= 0) return false;
    utf8.resize(len);
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(),
                        &utf8[0], len, NULL, NULL);
    return WriteBin(path, utf8.data(), (DWORD)utf8.size());
}

static std::wstring EnsureDir(const std::wstring& parent, const std::wstring& sub) {
    std::wstring path = parent + L"\\" + sub;
    CreateDirectoryW(path.c_str(), NULL);
    return path;
}

static std::wstring ResFilename(const std::wstring& name, WORD lang) {
    wchar_t buf[128];
    if (!name.empty() && name[0] == L'#') {
        int id = std::stoi(name.substr(1));
        wsprintfW(buf, L"ID_%04X_lang_%04X", id, lang);
    } else if (!name.empty()) {
        std::wstring s = name;
        for (auto& c : s)
            if (wcschr(L"\\/:*?\"<>|", c)) c = L'_';
        wsprintfW(buf, L"%s_lang_%04X", s.c_str(), lang);
    } else {
        wsprintfW(buf, L"unnamed_lang_%04X", lang);
    }
    return buf;
}

// -----------------------------------------------------------------------
// RT_STRING parser
// -----------------------------------------------------------------------
static std::wstring ParseStrings(const BYTE* data, DWORD size, WORD resId) {
    std::wstring out;
    wchar_t line[4096];
    DWORD off = 0;
    int blockBase = (int)resId * 16;

    while (off + 32 <= size) {
        WORD lens[16];
        memcpy(lens, data + off, 32);
        off += 32;
        for (int i = 0; i < 16; i++) {
            if (lens[i] > 0) {
                int bytes = lens[i] * 2;
                if (off + bytes > size) break;
                std::wstring s((const wchar_t*)(data + off), lens[i]);
                off += bytes;
                wsprintfW(line, L"ID=%d (0x%04X): %s\r\n",
                          blockBase + i, blockBase + i, s.c_str());
                out += line;
            }
        }
        blockBase += 16;
    }
    return out;
}

// -----------------------------------------------------------------------
// RT_DIALOG parser
// -----------------------------------------------------------------------
static std::wstring ParseDialog(const BYTE* data, DWORD size) {
    std::wstring out;
    if (size < 18) return L"(data too small for DLG_TEMPLATE)\r\n";

    DWORD off = 0;
    DWORD style = *(const DWORD*)(data + off); off += 4;
    DWORD exSty = *(const DWORD*)(data + off); off += 4;
    WORD cItems = *(const WORD*)(data + off); off += 2;
    short x = *(const short*)(data + off); off += 2;
    short y = *(const short*)(data + off); off += 2;
    short cx = *(const short*)(data + off); off += 2;
    short cy = *(const short*)(data + off); off += 2;

    auto readStr = [&](std::wstring* s) {
        if (off + 2 > size) return false;
        WORD val = *(const WORD*)(data + off);
        if (val == 0xFFFF) {
            off += 2;
            if (off + 2 > size) return false;
            WORD ord = *(const WORD*)(data + off); off += 2;
            if (s) *s = L"#" + std::to_wstring(ord);
            return true;
        }
        if (val == 0) { off += 2; if (s) *s = L""; return true; }
        DWORD end = off;
        while (end + 2 <= size && *(const WORD*)(data + end) != 0) end += 2;
        if (end + 2 > size) end = size;
        DWORD cc = (end - off) / 2;
        if (s) *s = (cc > 0) ? std::wstring((const wchar_t*)(data + off), cc) : L"";
        off = end + 2;
        return true;
    };

    std::wstring menuStr, clsStr, titleStr;
    readStr(&menuStr); readStr(&clsStr); readStr(&titleStr);

    wchar_t line[1024];
    wsprintfW(line, L"Style=0x%08X ExStyle=0x%08X Rect=(%d,%d,%d,%d) Items=%u\r\n",
              style, exSty, x, y, cx, cy, cItems);
    out += line;
    if (!titleStr.empty()) { wsprintfW(line, L"Title: %s\r\n", titleStr.c_str()); out += line; }
    if (!menuStr.empty())  { wsprintfW(line, L"Menu: %s\r\n", menuStr.c_str()); out += line; }
    if (!clsStr.empty())   { wsprintfW(line, L"Class: %s\r\n", clsStr.c_str()); out += line; }

    if (style & DS_SETFONT) {
        if (off + 8 <= size) {
            WORD pts = *(const WORD*)(data + off); off += 2;
            WORD wt  = *(const WORD*)(data + off); off += 2;
            BYTE it  = data[off++];
            BYTE cs  = data[off++];
            std::wstring font;
            readStr(&font);
            wsprintfW(line, L"Font: %s (%upt, weight=%u, italic=%d, charset=%u)\r\n",
                      font.c_str(), pts, wt, (int)it, cs);
            out += line;
        }
    }

    for (WORD i = 0; i < cItems; i++) {
        off = (off + 3) & ~3;
        if (off + 18 > size) break;
        DWORD cs_ = *(const DWORD*)(data + off); off += 4;
        DWORD es_ = *(const DWORD*)(data + off); off += 4;
        short cx_ = *(const short*)(data + off); off += 2;
        short cy_ = *(const short*)(data + off); off += 2;
        short cw_ = *(const short*)(data + off); off += 2;
        short ch_ = *(const short*)(data + off); off += 2;
        WORD  id_ = *(const WORD*)(data + off); off += 2;

        std::wstring cCls, cTxt;
        readStr(&cCls); readStr(&cTxt);
        if (off < size) off++;

        wsprintfW(line, L"  [%u] ID=0x%04X (%d,%d,%d,%d) class=%s text=\"%s\" style=0x%08X\r\n",
                  i, id_, cx_, cy_, cw_, ch_, cCls.c_str(), cTxt.c_str(), cs_);
        out += line;
    }
    return out;
}

// -----------------------------------------------------------------------
// RT_MENU parser
// -----------------------------------------------------------------------
static std::wstring ParseMenu(const BYTE* data, DWORD size) {
    std::wstring out;
    if (size < 4) return L"(data too small)\r\n";
    DWORD off = 0;
    WORD ver = *(const WORD*)(data + off); off += 2;
    WORD help = *(const WORD*)(data + off); off += 2;
    wchar_t line[512];
    wsprintfW(line, L"Version=%u HelpID=%u\r\n", ver, help);
    out += line;

    auto readStr = [&](std::wstring* s) {
        if (off + 2 > size) return false;
        WORD val = *(const WORD*)(data + off);
        if (val == 0) { off += 2; *s = L""; return true; }
        DWORD end = off;
        while (end + 2 <= size && *(const WORD*)(data + end) != 0) end += 2;
        if (end + 2 > size) end = size;
        DWORD cc = (end - off) / 2;
        *s = (cc > 0) ? std::wstring((const wchar_t*)(data + off), cc) : L"";
        off = end + 2;
        return true;
    };

    while (off + 4 <= size) {
        DWORD flags = *(const DWORD*)(data + off); off += 4;
        std::wstring text;
        readStr(&text);
        if (flags == 0 && text.empty()) break;
        wsprintfW(line, L"  0x%08X \"%s\"\r\n", flags, text.c_str());
        out += line;
    }
    return out;
}

// -----------------------------------------------------------------------
// RT_ACCELERATOR parser
// -----------------------------------------------------------------------
static std::wstring ParseAccel(const BYTE* data, DWORD size) {
    std::wstring out;
    wchar_t line[128];
    DWORD off = 0;
    while (off + 8 <= size) {
        WORD f = *(const WORD*)(data + off); off += 2;
        WORD k = *(const WORD*)(data + off); off += 2;
        WORD i = *(const WORD*)(data + off); off += 2;
        off = (off + 1) & ~1;
        wsprintfW(line, L"  Flags=0x%04X Key=0x%04X Cmd=0x%04X\r\n", f, k, i);
        out += line;
        if (f & 0x80) break;
    }
    return out;
}

// -----------------------------------------------------------------------
// RT_VERSION parser
// -----------------------------------------------------------------------
static std::wstring ParseVersion(const BYTE* data, DWORD size) {
    std::wstring out;
    if (size < 34) return L"(data too small)\r\n";
    wchar_t line[1024];

    auto rstr = [&](DWORD o, std::wstring* s, DWORD* next) {
        if (o + 2 > size) return false;
        WORD len = *(const WORD*)(data + o);
        if (len == 0) { *s = L""; *next = o + 2; return true; }
        DWORD bytes = (DWORD)len * 2;
        if (o + 2 + bytes > size) return false;
        *s = std::wstring((const wchar_t*)(data + o + 2), len);
        *next = o + 2 + bytes;
        return true;
    };
    auto a4 = [](DWORD o) { return (o + 3) & ~3; };

    DWORD total = *(const WORD*)(data + 0);
    DWORD vlen  = *(const WORD*)(data + 2);
    WORD  type  = *(const WORD*)(data + 4);
    std::wstring key;
    DWORD off = 6;
    if (!rstr(off, &key, &off)) return L"(parse error at key)\r\n";
    off = a4(off);

    wsprintfW(line, L"Key=%s Type=%u ValueLen=%u TotalLen=%u\r\n", key.c_str(), type, vlen, total);
    out += line;

    if (vlen > 0 && type == 0 && off + vlen <= size && vlen >= 48) {
        const BYTE* vi = data + off;
        if (*(const DWORD*)vi == 0xFEEF04BD) {
            DWORD fvMS = *(const DWORD*)(vi + 4);
            DWORD fvLS = *(const DWORD*)(vi + 8);
            DWORD pvMS = *(const DWORD*)(vi + 12);
            DWORD pvLS = *(const DWORD*)(vi + 16);
            wsprintfW(line, L"FileVersion=%u.%u.%u.%u\r\n",
                      HIWORD(fvMS), LOWORD(fvMS), HIWORD(fvLS), LOWORD(fvLS));
            out += line;
            wsprintfW(line, L"ProductVersion=%u.%u.%u.%u\r\n",
                      HIWORD(pvMS), LOWORD(pvMS), HIWORD(pvLS), LOWORD(pvLS));
            out += line;
            wsprintfW(line, L"FileFlags=0x%08X FileOS=0x%08X FileType=0x%08X\r\n",
                      *(const DWORD*)(vi + 24), *(const DWORD*)(vi + 28),
                      *(const DWORD*)(vi + 32));
            out += line;
        }
        off += vlen;
        off = a4(off);
    }

    while (off + 8 <= size && off < total) {
        DWORD clen = *(const WORD*)(data + off);
        if (clen == 0) break;
        DWORD ckEnd = off + 6;
        std::wstring ck;
        if (!rstr(ckEnd, &ck, &ckEnd)) break;
        ckEnd = a4(ckEnd);
        DWORD cvOff = ckEnd;

        if (ck == L"StringFileInfo") {
            DWORD si = cvOff, siEnd = off + clen;
            while (si + 8 <= siEnd) {
                DWORD stLen = *(const WORD*)(data + si);
                if (stLen == 0) break;
                DWORD skEnd = si + 6;
                std::wstring sk;
                if (!rstr(skEnd, &sk, &skEnd)) break;
                skEnd = a4(skEnd);
                DWORD so = skEnd, se = si + stLen;
                while (so + 8 <= se) {
                    DWORD sl = *(const WORD*)(data + so);
                    if (sl == 0) break;
                    DWORD sk2End = so + 6;
                    std::wstring sk2;
                    if (!rstr(sk2End, &sk2, &sk2End)) break;
                    sk2End = a4(sk2End);
                    std::wstring sv;
                    rstr(sk2End, &sv, &sk2End);
                    wsprintfW(line, L"%s=%s\r\n", sk2.c_str(), sv.c_str());
                    out += line;
                    so += sl; so = a4(so);
                }
                si += stLen; si = a4(si);
            }
        } else if (ck == L"VarFileInfo") {
            DWORD vi = cvOff, viEnd = off + clen;
            while (vi + 8 <= viEnd) {
                DWORD vl = *(const WORD*)(data + vi);
                if (vl == 0) break;
                DWORD vkEnd = vi + 6;
                std::wstring vk;
                if (!rstr(vkEnd, &vk, &vkEnd)) break;
                vkEnd = a4(vkEnd);
                if (vk == L"Translation") {
                    for (DWORD t = vkEnd; t + 4 <= vi + vl; t += 4) {
                        wsprintfW(line, L"Translation=0x%04X/0x%04X\r\n",
                                  *(const WORD*)(data + t), *(const WORD*)(data + t + 2));
                        out += line;
                    }
                }
                vi += vl; vi = a4(vi);
            }
        }
        off += clen; off = a4(off);
    }
    return out;
}

// -----------------------------------------------------------------------
// Dispatcher
// -----------------------------------------------------------------------
static bool IsPng(const BYTE* d, DWORD s) {
    return s >= 8 && d[0] == 0x89 && d[1] == 0x50 && d[2] == 0x4E && d[3] == 0x47;
}

struct Ctx {
    std::wstring outDir;
    int files;
    std::wstring curTypeName;
    WORD curTypeId;
};

static std::wstring Sanitize(const std::wstring& s) {
    std::wstring r = s;
    for (auto& c : r)
        if (wcschr(L"\\/:*?\"<>|", c)) c = L'_';
    return r;
}

static std::wstring TypeDirName(const std::wstring& typeName, WORD typeId) {
    wchar_t buf[128];
    if (typeId != 0) {
        wsprintfW(buf, L"type_%u_%s", typeId, GetTypeName(typeId));
    } else {
        wsprintfW(buf, L"type_str_%s", Sanitize(typeName).c_str());
    }
    return buf;
}

static void Process(Ctx& ctx, const std::wstring& typeName, WORD typeId,
                    const std::wstring& name, WORD lang,
                    const BYTE* data, DWORD size)
{
    std::wstring dir = EnsureDir(ctx.outDir, TypeDirName(typeName, typeId));
    std::wstring base = ResFilename(name, lang);

    switch (typeId) {
    case 6: { // STRING
        int id = std::stoi(name.substr(1));
        auto txt = ParseStrings(data, size, id);
        auto path = dir + L"\\" + base + L"_strings.txt";
        if (WriteUtf8(path, txt)) {
            ctx.files++;
        }
        break;
    }
    case 5: {
        auto txt = ParseDialog(data, size);
        auto path = dir + L"\\" + base + L"_dialog.txt";
        if (WriteUtf8(path, txt)) { ctx.files++; }
        break;
    }
    case 4: {
        auto txt = ParseMenu(data, size);
        auto path = dir + L"\\" + base + L"_menu.txt";
        if (WriteUtf8(path, txt)) { ctx.files++; }
        break;
    }
    case 9: {
        auto txt = ParseAccel(data, size);
        auto path = dir + L"\\" + base + L"_accelerator.txt";
        if (WriteUtf8(path, txt)) { ctx.files++; }
        break;
    }
    case 16: {
        auto txt = ParseVersion(data, size);
        auto path = dir + L"\\version.ini";
        if (WriteUtf8(path, txt)) { ctx.files++; }
        break;
    }
    case 3: case 14: {
        auto path = dir + L"\\" + base + L".ico";
        if (WriteBin(path, data, size)) { ctx.files++; }
        break;
    }
    case 1: case 12: {
        auto path = dir + L"\\" + base + L".cur";
        if (WriteBin(path, data, size)) { ctx.files++; }
        break;
    }
    case 2: {
        auto path = dir + L"\\" + base + L".bmp";
        if (WriteBin(path, data, size)) { ctx.files++; }
        break;
    }
    case 23: {
        auto path = dir + L"\\" + base + L".html";
        if (WriteBin(path, data, size)) { ctx.files++; }
        break;
    }
    case 24: {
        auto path = dir + L"\\" + base + L".xml";
        if (WriteBin(path, data, size)) { ctx.files++; }
        break;
    }
    default: {
        auto ext = IsPng(data, size) ? L"png" : L"bin";
        auto path = dir + L"\\" + base + L"." + ext;
        if (WriteBin(path, data, size)) { ctx.files++; }
        break;
    }
    }
}

// -----------------------------------------------------------------------
// Enum callbacks
// -----------------------------------------------------------------------
static BOOL CALLBACK EnumLangs(HMODULE hMod, LPCWSTR type, LPCWSTR name,
                               WORD lang, LONG_PTR param)
{
    Ctx& ctx = *reinterpret_cast<Ctx*>(param);
    HRSRC hRes = FindResourceExW(hMod, type, name, lang);
    if (!hRes) return TRUE;
    HGLOBAL hGlob = LoadResource(hMod, hRes);
    if (!hGlob) return TRUE;
    DWORD size = SizeofResource(hMod, hRes);
    const BYTE* data = (const BYTE*)LockResource(hGlob);
    if (!data || size == 0) return TRUE;

    std::wstring nameStr;
    if (IS_INTRESOURCE(name)) {
        wchar_t buf[32]; wsprintfW(buf, L"#%u", (WORD)(ULONG_PTR)name);
        nameStr = buf;
    } else {
        nameStr = name;
    }

    Process(ctx, ctx.curTypeName, ctx.curTypeId, nameStr, lang, data, size);
    return TRUE;
}

static BOOL CALLBACK EnumNames(HMODULE hMod, LPCWSTR type, LPWSTR name,
                               LONG_PTR param)
{
    Ctx& ctx = *reinterpret_cast<Ctx*>(param);
    // Ensure curTypeId and curTypeName are set before entering EnumLangs
    if (IS_INTRESOURCE(type)) {
        ctx.curTypeId = (WORD)(ULONG_PTR)type;
        ctx.curTypeName = GetTypeName(ctx.curTypeId);
    } else {
        ctx.curTypeId = 0;
        ctx.curTypeName = type;
    }
    EnumResourceLanguagesW(hMod, type, name, EnumLangs, param);
    return TRUE;
}

static BOOL CALLBACK EnumTypes(HMODULE hMod, LPWSTR type, LONG_PTR param) {
    EnumResourceNamesW(hMod, type, EnumNames, param);
    return TRUE;
}

// -----------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------
int wmain(int argc, wchar_t* argv[]) {
    if (argc < 3) {
        fprintf(stderr,
            "Usage: resource_extractor.exe <dll_path> <output_dir>\n"
            "\n"
            "Extracts all Win32 resources from a PE resource DLL.\n"
            "Output is organized into type_<ID>_<NAME>/ subdirectories.\n"
            "Handles: STRING, DIALOG, MENU, ACCELERATOR, VERSION,\n"
            "         ICON, CURSOR, BITMAP, HTML, MANIFEST, RCDATA,\n"
            "         PNG images, and custom resource types.\n");
        return 1;
    }

    LPCWSTR dllPath = argv[1];
    LPCWSTR outDir = argv[2];

    wprintf(L"Resource Extractor v1.0\n");
    wprintf(L"  Input:  %s\n", dllPath);
    wprintf(L"  Output: %s\n", outDir);

    CreateDirectoryW(outDir, NULL);

    HMODULE hMod = LoadLibraryExW(dllPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (!hMod) {
        wprintf(L"Error: LoadLibraryEx failed (error %u)\n", GetLastError());
        wprintf(L"  Check that the file exists and is a valid PE resource DLL.\n");
        return 1;
    }

    Ctx ctx = { outDir, 0 };

    if (!EnumResourceTypesW(hMod, EnumTypes, (LONG_PTR)&ctx)) {
        wprintf(L"Warning: EnumResourceTypes failed\n");
    }

    FreeLibrary(hMod);
    wprintf(L"\nDone: %d resource files written to %s\n", ctx.files, outDir);
    return 0;
}
