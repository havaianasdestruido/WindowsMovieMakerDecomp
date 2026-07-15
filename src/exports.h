#pragma once
#ifndef WMMR_EXPORTS_H
#define WMMR_EXPORTS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WMMR_DLL_EXPORTS
    #define MOVIECORE_API __declspec(dllexport)
#else
    #define MOVIECORE_API __declspec(dllimport)
#endif

#ifdef WMMR_DLL_EXPORTS
    #define RIBBON_API __declspec(dllexport)
#else
    #define RIBBON_API __declspec(dllimport)
#endif

#ifdef WLXPHOTOBASE_EXPORTS
    #define WLXPHOTOBASE_API __declspec(dllexport)
#else
    #define WLXPHOTOBASE_API __declspec(dllimport)
#endif

#ifdef WLXMOVIELIBRARY_EXPORTS
    #define WLXMOVIELIBRARY_API __declspec(dllexport)
#else
    #define WLXMOVIELIBRARY_API __declspec(dllimport)
#endif

#ifdef WLXVIDEOTRIM_EXPORTS
    #define WLXVIDEOTRIM_API __declspec(dllexport)
#else
    #define WLXVIDEOTRIM_API __declspec(dllimport)
#endif

#ifdef WLXPIPELINE_EXPORTS
    #define WLXPIPELINE_API __declspec(dllexport)
#else
    #define WLXPIPELINE_API __declspec(dllimport)
#endif

#ifdef WLXPIPETRAN_EXPORTS
    #define WLXPIPETRAN_API __declspec(dllexport)
#else
    #define WLXPIPETRAN_API __declspec(dllimport)
#endif

#ifdef WLXMEDIAPUBLISHSUBSCRIBE_EXPORTS
    #define WLXMEDIAPUBLISHSUBSCRIBE_API __declspec(dllexport)
#else
    #define WLXMEDIAPUBLISHSUBSCRIBE_API __declspec(dllimport)
#endif

#ifdef WLXSLIDESHOW_EXPORTS
    #define WLXSLIDESHOW_API __declspec(dllexport)
#else
    #define WLXSLIDESHOW_API __declspec(dllimport)
#endif

#ifdef WLXPHOTOCINEMATIC_EXPORTS
    #define WLXPHOTOCINEMATIC_API __declspec(dllexport)
#else
    #define WLXPHOTOCINEMATIC_API __declspec(dllimport)
#endif

#ifdef WLXFACERECOGNITION_EXPORTS
    #define WLXFACERECOGNITION_API __declspec(dllexport)
#else
    #define WLXFACERECOGNITION_API __declspec(dllimport)
#endif

#ifdef WLXCODECHOST_EXPORTS
    #define WLXCODECHOST_API __declspec(dllexport)
#else
    #define WLXCODECHOST_API __declspec(dllimport)
#endif

#ifdef WLXTRANSCODE_EXPORTS
    #define WLXTRANSCODE_API __declspec(dllexport)
#else
    #define WLXTRANSCODE_API __declspec(dllimport)
#endif

#ifdef WLXMP4PARSER_EXPORTS
    #define WLXMP4PARSER_API __declspec(dllexport)
#else
    #define WLXMP4PARSER_API __declspec(dllimport)
#endif

#ifdef WLMFREADWRITE_EXPORTS
    #define WLMFREADWRITE_API __declspec(dllexport)
#else
    #define WLMFREADWRITE_API __declspec(dllimport)
#endif

#ifdef WLMFDS_EXPORTS
    #define WLMFDS_API __declspec(dllexport)
#else
    #define WLMFDS_API __declspec(dllimport)
#endif

#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)

#ifdef __cplusplus
}
#endif

// MovieMakerCore.dll exports
#ifdef WMMR_DLL_EXPORTS
extern "C" {
    MOVIECORE_API int __cdecl MovieMakerMain(int argc, wchar_t** argv);
}
#endif

// WLXPhotoBase.dll exports (56 total)
#ifdef WLXPHOTOBASE_EXPORTS
namespace Base {
    class Exception;
    class String;
    class OS;
    class CPU;
    class Private;
}

extern "C" {
    WLXPHOTOBASE_API void __cdecl BaseThrow(const char* message);
    WLXPHOTOBASE_API void __cdecl BaseThrowLastError();
    WLXPHOTOBASE_API long __cdecl BaseGdiplusStatusToHresult(int status);
    WLXPHOTOBASE_API bool __cdecl BaseOSIsWin8OrGreater();
    WLXPHOTOBASE_API bool __cdecl BaseOSIsWin7OrGreater();
    WLXPHOTOBASE_API void __cdecl BasePrivateDelete(void* ptr);
    WLXPHOTOBASE_API void* __cdecl BasePrivateNew(size_t size);
    WLXPHOTOBASE_API unsigned int __cdecl BaseCPUGetProcessorCount();
}

// C++ mangled exports (56 total)
// Base::Exception dtor
WLXPHOTOBASE_API void __cdecl ??1Exception@Base@@UEAA@XZ(void* thisPtr);

// Base::Exception::operator HRESULT
WLXPHOTOBASE_API long __cdecl ??BOCException@Base@@QEBA_JXZ(void* thisPtr);

// Base::String::GetBaseStringManager
WLXPHOTOBASE_API void* __cdecl ?GetBaseStringManager@String@Base@@QEAPEAXXZ();

// Base::Exception constructors (multiple overloads)
WLXPHOTOBASE_API void* __cdecl ??0Exception@Base@@QEAA@AEBV01@@Z(void* thisPtr, const void* other);
WLXPHOTOBASE_API void* __cdecl ??0Exception@Base@@QEAA@XZ(void* thisPtr);

// Base::Throw
WLXPHOTOBASE_API void __cdecl ?Throw@Base@@YAXAEBVException@1@@Z(const void* exception);

// Base::ThrowLastError
WLXPHOTOBASE_API void __cdecl ?ThrowLastError@Base@@YAXXZ();

// Base::GdiplusStatusToHresult
WLXPHOTOBASE_API long __cdecl ?GdiplusStatusToHresult@Base@@YAJW4GdiplusStatus@@@Z(int status);

// Base::OS::IsWin8OrGreater
WLXPHOTOBASE_API bool __cdecl ?IsWin8OrGreater@OS@Base@@SA_NXZ();

// Base::OS::IsWin7OrGreater
WLXPHOTOBASE_API bool __cdecl ?IsWin7OrGreater@OS@Base@@SA_NXZ();

// Base::Private::Delete
WLXPHOTOBASE_API void __cdecl ?Delete@Private@Base@@YAXPAX@Z(void* ptr);

// Base::Private::New
WLXPHOTOBASE_API void* __cdecl ?New@Private@Base@@SAPEAX_K@Z(size_t size);

// Base::CPU::GetProcessorCount
WLXPHOTOBASE_API unsigned int __cdecl ?GetProcessorCount@CPU@Base@@SAIXZ();

// ATL::BaseAtlThrow
WLXPHOTOBASE_API void __cdecl ?_ AtlThrow@ATL@@YAXJ@Z(long hr);
#endif

#endif // WMMR_EXPORTS_H
