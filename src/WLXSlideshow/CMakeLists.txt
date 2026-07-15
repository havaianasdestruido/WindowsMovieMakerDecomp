# WLXSlideshow.dll  --  Slideshow generation
# Version 16.4.3528.0331
# 4 COM exports (DllCanUnloadNow, DllGetClassObject, DllRegisterServer, DllUnregisterServer)

set(TARGET WLXSlideshow)

add_library(${TARGET} SHARED
    dllmain.cpp
    WLXSlideshow.cpp
    WLXSlideshow.def
)

set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME         "WLXSlideshow"
    PREFIX              ""
    SUFFIX              ".dll"
    WIN32_EXECUTABLE    NO
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    VERSION             "16.4.3528.331"
    PRODUCT_VERSION     "16.4.3528.331"
)

target_compile_definitions(${TARGET} PRIVATE
    WLXSLIDESHOW_EXPORTS
    _WINDLL
    _CRT_SECURE_NO_WARNINGS
    _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
    _ATL_ALL_WARNINGS
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
    d3d9
    d3dcompiler
    shlwapi
    msi
    WLXPhotoBase
)

set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

install(TARGETS ${TARGET}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
