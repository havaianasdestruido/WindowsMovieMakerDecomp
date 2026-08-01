#include <windows.h>
#include <iostream>

int main() {
    HMODULE lib = LoadLibraryA("MovieMakerCore.dll");
    if (!lib) {
        std::cerr << "LoadLibrary failed: " << GetLastError() << std::endl;
        return 1;
    }
    using Func = int (*)();
    Func movieMakerMain = (Func)GetProcAddress(lib, "MovieMakerMain");
    if (!movieMakerMain) {
        std::cerr << "GetProcAddress failed: " << GetLastError() << std::endl;
        FreeLibrary(lib);
        return 1;
    }
    std::cout << "Calling MovieMakerMain..." << std::endl;
    int ret = movieMakerMain();
    std::cout << "MovieMakerMain returned " << ret << std::endl;
    FreeLibrary(lib);
    return 0;
}
