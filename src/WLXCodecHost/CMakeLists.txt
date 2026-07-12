# WLXCodecHost.exe  --  Out-of-process codec host for decoding media
# Version 16.4.3528.0331
# 0 exports, 124 imports (gdiplus, WLXPhotoBase, MetadataSys)

set(TARGET WLXCodecHost)

add_executable(${TARGET} WIN32
    WLXCodecHost.cpp
)

set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME         "WLXCodecHost"
    WIN32_EXECUTABLE    YES
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    VERSION             "16.4.3528.331"
    PRODUCT_VERSION     "16.4.3528.331"
)

target_compile_definitions(${TARGET} PRIVATE
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

target_include_directories(${TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/WLXPhotoBase
)

target_link_libraries(${TARGET} PRIVATE
    kernel32
    user32
    gdi32
    advapi32
    ole32
    oleaut32
    gdiplus
    shlwapi
    rpcrt4
    WLXPhotoBase
)

set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS ${TARGET}
    RUNTIME DESTINATION bin
)
