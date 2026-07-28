#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static const char *exe_rel = "undecomp\\Photo Gallery\\WLXVideoAcquireWizard.exe";
static const char *dll_rel = "undecomp\\Photo Gallery\\WLXVideoAcquireWizardResources.dll";

void test_file_attributes(const char *exe_path, const char *dll_path) {
    printf("\n=== File Attribute Tests ===\n");

    WIN32_FILE_ATTRIBUTE_DATA info;
    TEST("EXE exists", GetFileAttributesExA(exe_path, GetFileExInfoStandard, &info));
    if (GetFileAttributesExA(exe_path, GetFileExInfoStandard, &info)) {
        ULARGE_INTEGER sz;
        sz.LowPart = info.nFileSizeLow;
        sz.HighPart = info.nFileSizeHigh;
        TEST("EXE size ~850KB (840K-900K)", sz.QuadPart > 840000 && sz.QuadPart < 900000);
        printf("  INFO: EXE size = %llu bytes (%.2f KB)\n", sz.QuadPart, sz.QuadPart / 1024.0);
    }

    TEST("DLL exists", GetFileAttributesExA(dll_path, GetFileExInfoStandard, &info));
    if (GetFileAttributesExA(dll_path, GetFileExInfoStandard, &info)) {
        ULARGE_INTEGER sz;
        sz.LowPart = info.nFileSizeLow;
        sz.HighPart = info.nFileSizeHigh;
        TEST("DLL size ~106KB (100K-115K)", sz.QuadPart > 100000 && sz.QuadPart < 115000);
        printf("  INFO: DLL size = %llu bytes (%.2f KB)\n", sz.QuadPart, sz.QuadPart / 1024.0);
    }
}

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
    TEST("Subsystem is Windows GUI", nt->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI);
    TEST("Linker >= 11.0", nt->OptionalHeader.MajorLinkerVersion >= 11);
    TEST("Image base 0x400000", nt->OptionalHeader.ImageBase == 0x00400000);
    TEST("Is executable", fh->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE);
    TEST("Is 32-bit machine", fh->Machine == IMAGE_FILE_MACHINE_I386);
    TEST("Has relocs not stripped", !(fh->Characteristics & IMAGE_FILE_RELOCS_STRIPPED));
    TEST("Entry point = 0x441E3", nt->OptionalHeader.AddressOfEntryPoint == 0x441E3);
    TEST("Size of image ~872KB", nt->OptionalHeader.SizeOfImage > 0xD0000 && nt->OptionalHeader.SizeOfImage < 0xE0000);
    TEST("Stack reserve = 512KB", nt->OptionalHeader.SizeOfStackReserve == 0x80000);
    TEST("Stack commit = 64KB", nt->OptionalHeader.SizeOfStackCommit == 0x10000);
    TEST("Heap reserve = 1MB", nt->OptionalHeader.SizeOfHeapReserve == 0x100000);
    TEST("Heap commit = 4KB", nt->OptionalHeader.SizeOfHeapCommit == 0x1000);

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
    TEST("Number of sections = 4", fh->NumberOfSections == 4);
    TEST("Has .text section", found_text);
    TEST("Has .rsrc section", found_rsrc);
    TEST("Has .data section", found_data);
    TEST("Has .reloc section", found_reloc);

    UnmapViewOfFile(base);
    CloseHandle(hMapping);
    CloseHandle(hFile);
}

void test_load_library_exe(const char *exe_path) {
    printf("\n=== LoadLibrary Tests (EXE) ===\n");

    HMODULE hMod = LoadLibraryExA(exe_path, NULL, DONT_RESOLVE_DLL_REFERENCES);
    TEST("LoadLibraryEx(DONT_RESOLVE_DLL_REFERENCES)", hMod != NULL);
    if (hMod) {
        PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)hMod;
        TEST("DOS header at base", dos->e_magic == IMAGE_DOS_SIGNATURE);
        FreeLibrary(hMod);
    }

    hMod = LoadLibraryExA(exe_path, NULL, LOAD_LIBRARY_AS_DATAFILE);
    TEST("LoadLibraryEx(AS_DATAFILE)", hMod != NULL);
    if (hMod) {
        HMODULE base = (HMODULE)((DWORD_PTR)hMod - 1);
        PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
        TEST("DOS header via datafile", dos->e_magic == IMAGE_DOS_SIGNATURE);
        FreeLibrary(hMod);
    }

    hMod = LoadLibraryExA(exe_path, NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    TEST("LoadLibraryEx(AS_IMAGE_RESOURCE)", hMod != NULL);
    if (hMod) {
        HMODULE base = (HMODULE)((DWORD_PTR)hMod - 2);
        PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
        TEST("DOS header via image resource", dos->e_magic == IMAGE_DOS_SIGNATURE);
        FreeLibrary(hMod);
    }
}

void test_resources_dll(const char *dll_path) {
    printf("\n=== Resources DLL Tests ===\n");

    HMODULE hMod = LoadLibraryExA(dll_path, NULL, LOAD_LIBRARY_AS_DATAFILE);
    TEST("Load Resources DLL as datafile", hMod != NULL);
    if (hMod) {
        HMODULE base = (HMODULE)((DWORD_PTR)hMod - 1);
        PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
        TEST("DOS header valid", dos->e_magic == IMAGE_DOS_SIGNATURE);

        PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE*)base + dos->e_lfanew);
        TEST("NT signature valid", nt->Signature == IMAGE_NT_SIGNATURE);

        DWORD rsrc = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;
    TEST("Has resource data > 85KB", rsrc > 87000);
    printf("  INFO: Resource directory size = %lu bytes (%.2f KB)\n", rsrc, rsrc / 1024.0);

        TEST("Image base is 0x10000000", nt->OptionalHeader.ImageBase == 0x10000000);
        TEST("Entry point is 0 (resource-only)", nt->OptionalHeader.AddressOfEntryPoint == 0);
        TEST("Subsystem is CUI (3)", nt->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI);

        int found = 0;
        PIMAGE_SECTION_HEADER sec = IMAGE_FIRST_SECTION(nt);
        for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
            char name[9] = {};
            memcpy(name, sec[i].Name, 8);
            if (!strcmp(name, ".rsrc")) { found = 1; break; }
        }
        TEST("Only 1 section (resource-only DLL)", nt->FileHeader.NumberOfSections == 1);
        TEST("Section is .rsrc", found);

        FreeLibrary(hMod);
    }
}

void test_create_process(const char *exe_path) {
    printf("\n=== CreateProcess Tests ===\n");

    char cmdline[1024];
    sprintf(cmdline, "\"%s\"", exe_path);

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    BOOL ok = CreateProcessA(NULL, cmdline, NULL, NULL, FALSE,
                             CREATE_SUSPENDED, NULL, NULL, &si, &pi);
    TEST("CreateProcess(suspended) succeeds", ok != 0);
    if (ok) {
        DWORD ec = STILL_ACTIVE;
        GetExitCodeProcess(pi.hProcess, &ec);
        TEST("Process still active while suspended", ec == STILL_ACTIVE);

        TerminateProcess(pi.hProcess, 1);
        WaitForSingleObject(pi.hProcess, 5000);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

    sprintf(cmdline, "\"%s\" --help", exe_path);
    ok = CreateProcessA(NULL, cmdline, NULL, NULL, FALSE,
                        CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (ok) {
        WaitForSingleObject(pi.hProcess, 10000);
        DWORD ec = 0xFFFF;
        GetExitCodeProcess(pi.hProcess, &ec);
        printf("  INFO: --help exit code = %u (0x%08lX)\n", ec, ec);
        TEST("Process launched with --help", ec != 0xFFFF);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    } else {
        printf("  INFO: --help launch failed (expected for GUI-only EXE)\n");
    }

    sprintf(cmdline, "\"%s\" /AutoPlay", exe_path);
    ok = CreateProcessA(NULL, cmdline, NULL, NULL, FALSE,
                        CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (ok) {
        WaitForSingleObject(pi.hProcess, 10000);
        DWORD ec = 0xFFFF;
        GetExitCodeProcess(pi.hProcess, &ec);
        printf("  INFO: /AutoPlay exit code = %u (0x%08lX)\n", ec, ec);
        TEST("Process launched with /AutoPlay", ec != 0xFFFF);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    } else {
        printf("  INFO: /AutoPlay launch failed\n");
    }
}

int main(void) {
    printf("WLXVideoAcquireWizard.exe - Test Harness\n");
    printf("=========================================\n\n");

    char exe_path[MAX_PATH], dll_path[MAX_PATH];
    GetFullPathNameA(exe_rel, MAX_PATH, exe_path, NULL);
    GetFullPathNameA(dll_rel, MAX_PATH, dll_path, NULL);

    printf("EXE: %s\n", exe_path);
    printf("DLL: %s\n\n", dll_path);

    test_file_attributes(exe_path, dll_path);
    test_pe_headers(exe_path);
    test_load_library_exe(exe_path);
    test_resources_dll(dll_path);
    test_create_process(exe_path);

    printf("\n=========================================\n");
    printf("Results: %d passed, %d failed, %d total\n",
           g_passed, g_failed, g_passed + g_failed);
    return g_failed > 0 ? 1 : 0;
}
