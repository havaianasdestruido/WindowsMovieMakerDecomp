set(TARGET MetadataSys)

add_library(${TARGET} SHARED dllmain.cpp MetadataSys.cpp MetadataSys.def)

set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME "MetadataSys"
    PREFIX ""
    SUFFIX ".dll"
    WIN32_EXECUTABLE NO
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    VERSION "16.4.3528.331"
    PRODUCT_VERSION "16.4.3528.331"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

target_compile_definitions(${TARGET} PRIVATE
    _WINDLL
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
    gdiplus
    advapi32
    ole32
    oleaut32
    shlwapi
    shell32
    propsys
    mfplat
    mf
    mfreadwrite
    mfuuid
    WLXPhotoBase
)

install(TARGETS ${TARGET}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
