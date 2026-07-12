# MovieMakerCore.dll  --  Core engine for Windows Live Movie Maker 2012
# Version 16.4.3528.0331
# Single export: MovieMakerMain
# Codename: Sundance

set(TARGET MovieMakerCore)

add_library(${TARGET} SHARED
    dllmain.cpp
    MovieMakerCore.cpp
    MovieMakerCore.def
    pch.cpp
    SundanceApp/SundanceAppMain.cpp
    SundanceApp/SundanceAppDataContext.cpp
    SundanceApp/CommandLineParser.cpp
    SundanceApp/AutoSaveManager.cpp
    SundanceApp/ClipboardManager.cpp
    SundanceApp/MediaBrowser.cpp
    StoryboardManager/StoryboardManager.cpp
    StoryboardManager/MovieProject.cpp
    StoryboardManager/MovieExtent.cpp
    StoryboardManager/Extents.cpp
    StoryboardManager/Templates.cpp
    StoryboardManager/TimelineTrack.cpp
    StoryboardManager/StoryboardManagerExceptions.cpp
    HMRAVSource/HMRAVSource.cpp
    HMRAVSource/AVSource.cpp
    HMRAVSource/MFSource.cpp
    HMRAVSource/DShowSource.cpp
    HMRAVSource/EncodeProfile.cpp
    HMRAVSource/StreamSink.cpp
    HMRAVSource/TranscodeManager.cpp
    HMRAVSource/VideoCapture.cpp
    HMRAVSource/AudioCapture.cpp
    HMRAVSource/AVCaptureSession.cpp
    HMRAVSource/CachedWFSection.cpp
    DataStructs/IntSet.cpp
    DataStructs/StringSet.cpp
)

set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME         "MovieMakerCore"
    PREFIX              ""
    SUFFIX              ".dll"
    WIN32_EXECUTABLE    NO
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    VERSION             "16.4.3528.331"
    PRODUCT_VERSION     "16.4.3528.331"
)

target_compile_definitions(${TARGET} PRIVATE
    MOVIECORE_EXPORTS
    _WINDLL
    _AFXDLL
    _CRT_SECURE_NO_WARNINGS
    _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
    _ATL_ALL_WARNINGS
)

target_include_directories(${TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/WLXPhotoBase
)

# Direct system library links (imported at load time)
target_link_libraries(${TARGET} PRIVATE
    kernel32
    user32
    gdi32
    advapi32
    shell32
    shlwapi
    ole32
    oleaut32
    gdiplus
    d3d11
    d3d9
    d3dcompiler_46
    dxva2
    mfplat
    mf
    mfuuid
    mfreadwrite
    strmiids
    quartz
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
    uxctl
)

# Delay-loaded DLLs (resolved at runtime on first call)
set(DELAY_LOAD_LIBS
    WLXPhotoSqm
    DmxBici
    MetadataSys
    wlidcli
)

foreach(_dll ${DELAY_LOAD_LIBS})
    target_link_options(${TARGET} PRIVATE "/DELAYLOAD:${_dll}.dll")
endforeach()

# Link delay-load helper (for clean unload)
target_link_libraries(${TARGET} PRIVATE delayimp)

# Subsystem: WINDOWS (GUI application, not console)
set_target_properties(${TARGET} PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS /DELAYLOAD:WLXPhotoSqm.dll /DELAYLOAD:DmxBici.dll /DELAYLOAD:MetadataSys.dll /DELAYLOAD:wlidcli.dll"
)

# Output to bin/
set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

install(TARGETS ${TARGET}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
