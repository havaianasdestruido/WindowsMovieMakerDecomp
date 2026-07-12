# MovieMaker.exe  --  Thin launcher for Windows Live Movie Maker 2012
# Version 16.4.3528.0331

set(TARGET MovieMaker)

add_executable(${TARGET} WIN32
    main.cpp
    moviemaker.rc
)

set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME         "MovieMaker"
    WIN32_EXECUTABLE    YES
)

target_compile_definitions(${TARGET} PRIVATE
    _CRT_SECURE_NO_WARNINGS
)

target_include_directories(${TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies (matching original binary: kernel32 only)
target_link_libraries(${TARGET} PRIVATE
    kernel32
    WLXPhotoBase
)

# Static runtime (MSVCR110 equivalent for VS 11.0 / _MSC_VER 1700)
set_target_properties(${TARGET} PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded"
)

# Embed manifest as resource ID 2 (RT_MANIFEST).
# The RC file already includes "2 RT_MANIFEST ...", so we only need
# to set MANIFEST_FILE so CMake doesn't auto-generate a default one.
set(MANIFEST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/MovieMaker.exe.manifest")
if(EXISTS "${MANIFEST_FILE}")
    set_target_properties(${TARGET} PROPERTIES
        MANIFEST_FILE "${MANIFEST_FILE}"
    )
endif()

# Delay-load MovieMakerCore.dll via its import library.
# The .lib file should be produced by the MovieMakerCore subproject.
set(MOVIEMAKERCORE_LIB
    "${CMAKE_BINARY_DIR}/lib/MovieMakerCore.lib"
)
if(EXISTS "${MOVIEMAKERCORE_LIB}")
    target_link_libraries(${TARGET} PRIVATE
        "${MOVIEMAKERCORE_LIB}"
    )
    set_target_properties(${TARGET} PROPERTIES
        LINK_FLAGS "/DELAYLOAD:MovieMakerCore.dll"
    )
endif()

# Set version number
set_target_properties(${TARGET} PROPERTIES
    VERSION     "16.4.3528.331"
    PRODUCT_VERSION "16.4.3528.331"
)

# Output to bin/
set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
