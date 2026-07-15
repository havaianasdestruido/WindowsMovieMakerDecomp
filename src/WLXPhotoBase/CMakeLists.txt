cmake_minimum_required(VERSION 3.15)
project(WLXPhotoBase LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# WLXPhotoBase.dll - Shared foundation library for Windows Live Photo Gallery / Movie Maker
# This DLL provides base exception handling, memory management, OS version detection,
# GDI+ helpers, and common utilities used by all Photo Gallery and Movie Maker components.

add_library(WLXPhotoBase SHARED
    WLXPhotoBase.cpp
    $<TARGET_OBJECTS:BaseTypes>
)

set_target_properties(WLXPhotoBase PROPERTIES
    OUTPUT_NAME "WLXPhotoBase"
    PREFIX ""
    SUFFIX ".dll"
    # MSVC 11.0 (VS2012) compatible, target Windows 6.2+ (Win8+)
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    # Export the DLL version info
    VERSION 2012.1.0
)

target_include_directories(WLXPhotoBase PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/WTL
)

target_compile_definitions(WLXPhotoBase PRIVATE
    WINVER=0x0602
    _WIN32_WINNT=0x0602
    _WIN32_IE=0x0800
    UNICODE
    _UNICODE
    _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
    _ATL_ALL_WARNINGS
    WLXPHOTOBASE_EXPORTS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

target_link_libraries(WLXPhotoBase PRIVATE
    kernel32
    gdi32
    gdiplus
    advapi32
    user32
    shell32
    shlwapi
    wer
    psapi
    ole32
    oleaut32
    uuid
    comctl32
    comsuppw
)

# BaseTypes object library (header-only-ish templates compiled into the DLL)
add_library(BaseTypes OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/BaseTypes.h
)

set_target_properties(BaseTypes PROPERTIES
    LINKER_LANGUAGE CXX
)

target_include_directories(BaseTypes PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/WTL
)

target_compile_definitions(BaseTypes PRIVATE
    WINVER=0x0602
    _WIN32_WINNT=0x0602
    _WIN32_IE=0x0800
    UNICODE
    _UNICODE
    _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

install(TARGETS WLXPhotoBase
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
