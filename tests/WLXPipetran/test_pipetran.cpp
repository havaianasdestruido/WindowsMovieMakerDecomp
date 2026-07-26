#include <windows.h>
#include <cstdio>
#include <excpt.h>

typedef void* (__cdecl *PFN_GetTFXCreateFunctions)();

static int Filter(unsigned int code, struct _EXCEPTION_POINTERS *ep) {
    fprintf(stderr, "  SEH exception 0x%08X at EIP=0x%p\n", code, ep->ExceptionRecord->ExceptionAddress);
    return EXCEPTION_EXECUTE_HANDLER;
}

void ProbeAndTest(const wchar_t* path) {
    fprintf(stderr, "[*] Loading: %ls\n", path);
    HMODULE hMod = LoadLibraryW(path);
    if (!hMod) {
        fprintf(stderr, "  LoadLibrary failed: %u\n", GetLastError());
        return;
    }
    fprintf(stderr, "  Base: 0x%p\n", hMod);

    BYTE* base = (BYTE*)hMod;
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)base;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);

    fprintf(stderr, "  ImageBase: 0x%p  EntryPoint: 0x%X\n",
        (void*)nt->OptionalHeader.ImageBase, nt->OptionalHeader.AddressOfEntryPoint);
    fprintf(stderr, "  Sections (%u):\n", nt->FileHeader.NumberOfSections);
    IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        char sname[9] = {};
        memcpy(sname, sec[i].Name, 8);
        fprintf(stderr, "    %-8s VA=0x%08X VirtSize=0x%08X RawSize=0x%08X Flags=0x%08X\n",
            sname, sec[i].VirtualAddress, sec[i].Misc.VirtualSize,
            sec[i].SizeOfRawData, sec[i].Characteristics);
    }

    IMAGE_DATA_DIRECTORY& expDir = nt->OptionalHeader.DataDirectory[0];
    if (expDir.VirtualAddress) {
        IMAGE_EXPORT_DIRECTORY* exp = (IMAGE_EXPORT_DIRECTORY*)(base + expDir.VirtualAddress);
        fprintf(stderr, "  Exports: %u named, %u total\n",
            exp->NumberOfNames, exp->NumberOfFunctions);
        if (exp->NumberOfNames) {
            DWORD* names = (DWORD*)(base + exp->AddressOfNames);
            WORD* ords = (WORD*)(base + exp->AddressOfNameOrdinals);
            DWORD* funcs = (DWORD*)(base + exp->AddressOfFunctions);
            for (DWORD i = 0; i < exp->NumberOfNames; i++) {
                fprintf(stderr, "    [%u] %s -> RVA 0x%X\n",
                    ords[i], (const char*)(base + names[i]), funcs[ords[i]]);
            }
        }
    }

    PFN_GetTFXCreateFunctions pfn = (PFN_GetTFXCreateFunctions)GetProcAddress(hMod, "GetTFXCreateFunctions");
    fprintf(stderr, "  GetTFXCreateFunctions: 0x%p\n", pfn);

    if (pfn) {
        fprintf(stderr, "\n[*] Calling GetTFXCreateFunctions()...\n");
        __try {
            void* result = pfn();
            fprintf(stderr, "  Return value: 0x%p\n", result);
            if (result) {
                void* vtbl0 = *(void**)result;
                fprintf(stderr, "  vtable[0]: 0x%p (QueryInterface?)\n", vtbl0);
                void* vtbl1 = *((void**)result + 1);
                fprintf(stderr, "  vtable[1]: 0x%p (AddRef?)\n", vtbl1);
                void* vtbl2 = *((void**)result + 2);
                fprintf(stderr, "  vtable[2]: 0x%p (Release?)\n", vtbl2);
                fprintf(stderr, "  First 16 vtable entries:\n");
                for (int j = 0; j < 16; j++) {
                    fprintf(stderr, "    [%d] 0x%p\n", j, *((void**)result + j));
                }
            }
        } __except (Filter(GetExceptionCode(), GetExceptionInformation())) {
            fprintf(stderr, "  GetTFXCreateFunctions threw SEH\n");
        }
    }

    FreeLibrary(hMod);
    fprintf(stderr, "\n[*] Done.\n");
}

int main(int argc, char* argv[]) {
    const wchar_t* path = L"C:\\Users\\mcmco\\Desktop\\WMMR\\undecomp\\Photo Gallery\\WLXPipetran.dll";
    ProbeAndTest(path);
    return 0;
}
