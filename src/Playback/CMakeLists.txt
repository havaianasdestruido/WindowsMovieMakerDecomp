set(TARGET PlaybackEngine)

add_library(${TARGET} STATIC
    PlaybackEngine.cpp
    PlaybackEngine.h
)

set_target_properties(${TARGET} PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
)

target_include_directories(${TARGET} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/Renderer
)

target_link_libraries(${TARGET} PRIVATE
    kernel32
    user32
    d3d11
    dxgi
    mfplat
    mfreadwrite
    mfuuid
    ole32
    uuid
)

target_compile_definitions(${TARGET} PRIVATE
    _WINDLL
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)
