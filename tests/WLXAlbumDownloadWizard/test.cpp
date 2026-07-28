#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "imagehlp.lib")

static int g_passed = 0, g_failed = 0;

#define TEST(name, expr) do { \
    if (expr) { \
        printf("  PASS: %s\n", name); \
        g_passed++; \
    } else { \
        printf("  FAIL: %s\n", name); \
        g_failed++; \
    } \
} while(0)

#define CHECK(expr) do { if (!(expr)) { \
    printf("  FATAL: %s at %d\n", __FILE__, __LINE__); \
    exit(1); \
} } while(0)

static const char *exe_rel = "\\undecomp\\Photo Gallery\\WLXAlbumDownloadWizard.exe";
static const char *dll_rel = "\\undecomp\\Photo Gallery\\WLXAlbumDownloadWizardResources.dll";

void test_pe_headers(const char *exe_path) {
    printf("\n=== PE Header Tests ===\n");

    HANDLE hFile = CreateFileA(exe_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    CHECK(hFile != INVALID_HANDLE_VALUE);

    HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    CHECK(hMapping != NULL);

    LPVOID base = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    CHECK(base != NULL);

    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
    TEST("DOS signature valid", dos->e_magic == IMAGE_DOS_SIGNATURE);
    TEST("e_lfanew within file", dos->e_lfanew > 0 && dos->e_lfanew < 0x1000);

    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE*)base + dos->e_lfanew);
    TEST("NT signature valid", nt->Signature == IMAGE_NT_SIGNATURE);

    PIMAGE_FILE_HEADER fh = &nt->FileHeader;
    TEST("Machine is x86", fh->Machine == IMAGE_FILE_MACHINE_I386);
    TEST("Subsystem is GUI", nt->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI);
    TEST("Linker >= 11.0", nt->OptionalHeader.MajorLinkerVersion >= 11);
    TEST("Image base 0x400000", nt->OptionalHeader.ImageBase == 0x00400000);
    TEST("Is executable", fh->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE);
    TEST("Large Address Aware", fh->Characteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE);
    TEST("Has relocations", !(fh->Characteristics & IMAGE_FILE_RELOCS_STRIPPED));
    TEST("Entry point != 0", nt->OptionalHeader.AddressOfEntryPoint != 0);
    TEST("Size of image > 200KB", nt->OptionalHeader.SizeOfImage > 0x30000);
    TEST("Size of image < 1MB", nt->OptionalHeader.SizeOfImage < 0x100000);

    PIMAGE_SECTION_HEADER sec = IMAGE_FIRST_SECTION(nt);
    int found_text = 0, found_rsrc = 0, found_data = 0, found_reloc = 0;
    for (WORD i = 0; i < fh->NumberOfSections; i++) {
        char name[9] = {};
        memcpy(name, sec[i].Name, 8);
        if (!strcmp(name, ".text")) found_text = 1;
        if (!strcmp(name, ".rsrc")) found_rsrc = 1;
        if (!strcmp(name, ".data")) found_data = 1;
        if (!strcmp(name, ".reloc")) found_reloc = 1;
    }
    TEST("Has .text section", found_text);
    TEST("Has .rsrc section", found_rsrc);
    TEST("Has .data section", found_data);
    TEST("Has .reloc section", found_reloc);

    UnmapViewOfFile(base);
    CloseHandle(hMapping);
    CloseHandle(hFile);
}

void test_create_process(const char *exe_path) {
    printf("\n=== CreateProcess Tests ===\n");
    printf("  INFO: GUI subsystem; will test suspended launch only\n");

    char cmdline[1024];
    snprintf(cmdline, sizeof(cmdline), "\"%s\"", exe_path);

    STARTUPINFOA si = { 0 };
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = { 0 };
    BOOL ok = CreateProcessA(NULL, cmdline, NULL, NULL, FALSE,
                             CREATE_SUSPENDED, NULL, NULL, &si, &pi);
    TEST("CreateProcess (suspended) succeeds", ok != 0);
    if (ok) {
        DWORD ec = STILL_ACTIVE;
        GetExitCodeProcess(pi.hProcess, &ec);
        TEST("Process still active while suspended", ec == STILL_ACTIVE);

        TerminateProcess(pi.hProcess, 0);
        WaitForSingleObject(pi.hProcess, 2000);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

void test_load_library_exe(const char *exe_path) {
    printf("\n=== LoadLibrary Tests (EXE) ===\n");

    HMODULE hMod = LoadLibraryExA(exe_path, NULL, DONT_RESOLVE_DLL_REFERENCES);
    TEST("LoadLibraryEx(DONT_RESOLVE)", hMod != NULL);
    if (hMod) {
        PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)hMod;
        TEST("DOS header via DONT_RESOLVE", dos->e_magic == IMAGE_DOS_SIGNATURE);
        FreeLibrary(hMod);
    }

    hMod = LoadLibraryExA(exe_path, NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    TEST("LoadLibraryEx(AS_IMAGE_RESOURCE)", hMod != NULL);
    if (hMod) FreeLibrary(hMod);

    hMod = LoadLibraryExA(exe_path, NULL, LOAD_LIBRARY_AS_DATAFILE);
    TEST("LoadLibraryEx(AS_DATAFILE)", hMod != NULL);
    if (hMod) FreeLibrary(hMod);
}

void test_resources_dll(const char *dll_path) {
    printf("\n=== Resources DLL Tests ===\n");

    HMODULE hMod = LoadLibraryExA(dll_path, NULL, DONT_RESOLVE_DLL_REFERENCES);
    TEST("Load Resources DLL (DONT_RESOLVE)", hMod != NULL);
    if (hMod) {
        PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)hMod;
        TEST("DOS header valid", dos->e_magic == IMAGE_DOS_SIGNATURE);
        if (dos->e_magic == IMAGE_DOS_SIGNATURE) {
            PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE*)hMod + dos->e_lfanew);
            TEST("NT signature valid", nt->Signature == IMAGE_NT_SIGNATURE);
            if (nt->Signature == IMAGE_NT_SIGNATURE) {
                DWORD rsrc = (DWORD)nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;
                TEST("Has resource data > 1MB", rsrc > 0x100000);
                printf("  INFO: Resource directory size = %lu bytes (%.2f MB)\n", rsrc, rsrc / 1048576.0);
                TEST("Only .rsrc section", nt->FileHeader.NumberOfSections == 1);
            }
        }
        FreeLibrary(hMod);
    }

    hMod = LoadLibraryExA(dll_path, NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    TEST("Load Resources DLL as image resource", hMod != NULL);
    if (hMod) FreeLibrary(hMod);

    hMod = LoadLibraryExA(dll_path, NULL, LOAD_LIBRARY_AS_DATAFILE);
    TEST("Load Resources DLL as datafile", hMod != NULL);
    if (hMod) FreeLibrary(hMod);
}

void test_file_attributes(const char *exe_path, const char *dll_path) {
    printf("\n=== File Attribute Tests ===\n");

    WIN32_FILE_ATTRIBUTE_DATA info;
    TEST("EXE exists", GetFileAttributesExA(exe_path, GetFileExInfoStandard, &info));
    if (GetFileAttributesExA(exe_path, GetFileExInfoStandard, &info)) {
        ULARGE_INTEGER sz;
        sz.LowPart = info.nFileSizeLow;
        sz.HighPart = info.nFileSizeHigh;
        TEST("EXE size ~291KB", sz.QuadPart > 280000 && sz.QuadPart < 300000);
        printf("  INFO: EXE size = %llu bytes (%.2f KB)\n", sz.QuadPart, sz.QuadPart / 1024.0);
    }

    TEST("DLL exists", GetFileAttributesExA(dll_path, GetFileExInfoStandard, &info));
    if (GetFileAttributesExA(dll_path, GetFileExInfoStandard, &info)) {
        ULARGE_INTEGER sz;
        sz.LowPart = info.nFileSizeLow;
        sz.HighPart = info.nFileSizeHigh;
        TEST("DLL size ~1.4MB", sz.QuadPart > 1400000 && sz.QuadPart < 1600000);
        printf("  INFO: DLL size = %llu bytes (%.2f MB)\n", sz.QuadPart, sz.QuadPart / 1048576.0);
    }
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("WLXAlbumDownloadWizard.exe - Test Harness\n");
    printf("==========================================\n");

    char self[MAX_PATH], root[MAX_PATH], exe_path[MAX_PATH], dll_path[MAX_PATH];
    GetModuleFileNameA(NULL, self, MAX_PATH);
    strncpy(root, self, MAX_PATH);
    root[MAX_PATH - 1] = '\0';
    char *sep = strrchr(root, '\\');
    if (sep) { *sep = '\0'; sep = strrchr(root, '\\'); if (sep) *sep = '\0'; sep = strrchr(root, '\\'); if (sep) *sep = '\0'; }
    snprintf(exe_path, MAX_PATH, "%s%s", root, exe_rel);
    snprintf(dll_path, MAX_PATH, "%s%s", root, dll_rel);

    printf("EXE: %s\n", exe_path);
    printf("DLL: %s\n", dll_path);

    test_file_attributes(exe_path, dll_path);
    test_pe_headers(exe_path);
    test_load_library_exe(exe_path);
    test_resources_dll(dll_path);
    test_create_process(exe_path);

    printf("\n==========================================\n");
    printf("Results: %d passed, %d failed, %d total\n",
           g_passed, g_failed, g_passed + g_failed);
    return g_failed > 0 ? 1 : 0;
}
