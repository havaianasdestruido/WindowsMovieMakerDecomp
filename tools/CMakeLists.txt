cmake_minimum_required(VERSION 3.15)

project(resource_extractor
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Win32 resource extractor for DUI resource DLLs"
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR "This tool only builds on Windows.")
endif()

if(NOT MSVC)
    message(FATAL_ERROR "This tool requires the MSVC compiler.")
endif()

add_compile_definitions(
    WINVER=0x0601
    _WIN32_WINNT=0x0601
    _WINDOWS
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
    add_compile_options(/W4 /MP /GS /Zc:wchar_t /Zi /Od)
endif()

add_executable(resource_extractor resource_extractor.cpp)

target_link_libraries(resource_extractor PRIVATE
    kernel32
    user32
    shell32
    version
)
