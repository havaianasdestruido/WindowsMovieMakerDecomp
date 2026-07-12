# MovieMakerLang.dll  --  Resource-only satellite DLL for Windows Live Movie Maker 2012
# Version 16.4.3528.0331
# Contains all UI strings (201 RT_STRING tables), icons, cursors, PNG images, UIFILE resources.
# No code, no imports, no exports.

set(TARGET MovieMakerLang)

# Resource-only DLL: no source files, only an .rc resource script.
add_library(${TARGET} SHARED
    MovieMakerLang.rc
)

set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME         "MovieMakerLang"
    PREFIX              ""
    SUFFIX              ".dll"
    WIN32_EXECUTABLE    NO
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    VERSION             "16.4.3528.331"
    PRODUCT_VERSION     "16.4.3528.331"
)

target_compile_definitions(${TARGET} PRIVATE
    _CRT_SECURE_NO_WARNINGS
)

target_include_directories(${TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Resource-only DLL has no link dependencies (no code, no imports).
# CMake needs at least one dummy link to suppress "no object files" warning.
set_target_properties(${TARGET} PROPERTIES
    LINK_FLAGS "/NOENTRY"
)

# Output to bin/
set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS ${TARGET}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
