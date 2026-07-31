cmake_minimum_required(VERSION 3.15)

project(WMMR
    VERSION 16.4.3528.0331
    LANGUAGES CXX
    DESCRIPTION "Windows Live Movie Maker 2012 - Source Recreation"
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VERSION_MAJOR 16)
set(VERSION_MINOR 4)
set(VERSION_BUILD 3528)
set(VERSION_REVISION 331)

if(NOT WIN32)
    message(FATAL_ERROR "This project only builds on Windows with MSVC.")
endif()

if(NOT MSVC)
    message(FATAL_ERROR "This project requires the MSVC compiler.")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install prefix" FORCE)

set(COMMON_SYSTEM_LIBS
    kernel32
    user32
    gdi32
    shell32
    ole32
    oleaut32
    advapi32
    shlwapi
    gdiplus
    d3d11
    d3d9
    d3dcompiler
    dxva2
    mfplat
    mf
    dwmapi
    uxtheme
    version
    winmm
    XmlLite
    windowscodecs
    oleacc
    propsys
    esent
    d2d1
    dwrite
)

add_compile_definitions(
    WINVER=0x0601
    _WIN32_WINNT=0x0601
    _WINDOWS
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
    _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
    VERSION_MAJOR=${VERSION_MAJOR}
    VERSION_MINOR=${VERSION_MINOR}
    VERSION_BUILD=${VERSION_BUILD}
    VERSION_REVISION=${VERSION_REVISION}
)

if(MSVC)
    add_compile_options(
        /W4 /wd4005 /wd4100 /wd4505 /wd4251
        /MP
        /GS
        /Gy
        /guard:cf
        /Zc:forScope
        /Zc:wchar_t
        /Zi
        /Od
        /D_CRT_SECURE_NO_WARNINGS
    )
    add_link_options(
        /GUARD:CF
        /LTCG
        /OPT:REF
        /OPT:ICF
        /DEBUG
    )
endif()

add_subdirectory(src/WLXPhotoBase)
add_subdirectory(src/WLXMovieLibrary)
add_subdirectory(src/WLXVideoTrim)
add_subdirectory(src/WLXPipeline)
add_subdirectory(src/WLXPipetran)
add_subdirectory(src/WLXMediaPublishSubscribe)
add_subdirectory(src/WLXSlideshow)
add_subdirectory(src/WLXPhotoCinematic)
add_subdirectory(src/WLXFaceRecognition)
add_subdirectory(src/WLXCodecHost)
add_subdirectory(src/WLXTranscode)
add_subdirectory(src/WLXMP4Parser)
add_subdirectory(src/WLMFReadWrite)
add_subdirectory(src/WLMFDS)
add_subdirectory(src/MovieMakerCore)
add_subdirectory(src/MovieMakerLang)
add_subdirectory(src/MovieMakerPreviewClient)
add_subdirectory(src/MovieMaker)
add_subdirectory(src/DmxBici)
add_subdirectory(src/MetadataSys)
add_subdirectory(src/UXCore)
add_subdirectory(src/Project)
add_subdirectory(src/uxctl)
add_subdirectory(src/wlidcli)
add_subdirectory(src/Media)
add_subdirectory(src/WLXPhotoSqm)
add_subdirectory(src/Timeline)
add_subdirectory(src/Renderer)
add_subdirectory(src/Playback)
