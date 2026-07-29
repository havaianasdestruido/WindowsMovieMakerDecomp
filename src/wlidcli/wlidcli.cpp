#include <windows.h>

#define UNREFERENCED_PARAMETER(p) (p)

#define WLID_STUB_ORDINAL(ord) \
    extern "C" __declspec(dllexport) HRESULT __stdcall DummyFunc##ord() \
    { \
        UNREFERENCED_PARAMETER(ord); \
        return 0x80004005; \
    }

WLID_STUB_ORDINAL(2)
WLID_STUB_ORDINAL(3)
WLID_STUB_ORDINAL(8)
WLID_STUB_ORDINAL(29)
WLID_STUB_ORDINAL(41)
WLID_STUB_ORDINAL(108)
WLID_STUB_ORDINAL(113)
