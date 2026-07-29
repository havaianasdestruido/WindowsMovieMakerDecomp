set(TARGET UXCore)

add_library(${TARGET} SHARED
    UXCore.def
    dllmain.cpp
    UXCore.cpp
    Element.cpp
    Element.h
    Layout.cpp
    Layout.h
    Value.cpp
    Value.h
    Containers.h
    Containers.cpp
    Resources.h
    Resources.cpp
    Controls.h
    Controls.cpp
)

set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME "UXCore"
    PREFIX ""
    SUFFIX ".dll"
    WIN32_EXECUTABLE NO
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    STATIC_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
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
)

target_link_libraries(${TARGET} PRIVATE
    kernel32
    user32
    gdi32
    advapi32
    ole32
    oleaut32
    shlwapi
    comctl32
    uxtheme
    dwrite
    d2d1
    windowscodecs
)

target_link_options(${TARGET} PRIVATE /OPT:NOICF)

install(TARGETS ${TARGET}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
