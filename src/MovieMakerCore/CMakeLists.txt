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
    SundanceApp/ThumbnailCache.cpp
    SundanceApp/UndoManager.cpp
    SundanceApp/TimelineController.cpp
    SundanceApp/ProjectManager.cpp
    SundanceApp/ExportController.cpp
    SundanceApp/ImportController.cpp
    SundanceApp/PlaybackController.cpp
    StoryboardManager/StoryboardManager.cpp
    StoryboardManager/MovieProject.cpp
    StoryboardManager/MovieExtent.cpp
    StoryboardManager/Extents.cpp
    StoryboardManager/Templates.cpp
    StoryboardManager/TimelineTrack.cpp
    StoryboardManager/StoryboardManagerExceptions.cpp
    StoryboardManager/Theme/Theme.cpp
    StoryboardManager/Theme/ThemeTemplates.cpp
    StoryboardManager/Theme/ThemeStructure.cpp
    StoryboardManager/Theme/TextComposition.cpp
    StoryboardManager/MovieEffect/MovieEffect.cpp
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
    HMRAVSource/AVSourceFactory.cpp
    HMRAVSource/AVSourceProxy.cpp
    HMRAVSource/AVSink.cpp
    HMRAVSource/StreamSinkHost.cpp
    HMRAVSource/StreamSinkHelper.cpp
    HMRAVSource/SAXProfileBuilder.cpp
    HMRAVSource/MFRateControlHelper.cpp
    HMRAVSource/NativeMFSourceReaderBuilder.cpp
    HMRAVSource/AsyncSourceResolver.cpp
    HMRAVSource/DShowMFSourceReaderBuilder.cpp
    HMRAVSource/MFSourceReaderBuilder.cpp
    HMRAVSource/MFByteStreamOnStream.cpp
    HMRAVSource/VideoProc.cpp
    HMRAVSource/SyncVideoSource.cpp
    HMRAVSource/TextureInterop.cpp
    HMRAVSource/AuthProvider.cpp
    HMRAVSource/ImageThumbnail.cpp
    HMRAVSource/TranscodeMetadata.cpp
    HMRAVSource/Audio/Waveform.cpp
    HMRAVSource/Audio/AudioOutput.cpp
    HMRAVSource/Audio/AudioQueue.cpp
    HMRAVSource/Audio/Mixer.cpp
    HMRAVSource/Audio/AudioBoost.cpp
    HMRAVSource/Audio/AudioResampler.cpp
    HMRAVSource/Audio/AudioAnalysis.cpp
    DataStructs/IntSet.cpp
    DataStructs/StringSet.cpp
    UI/Ribbon/RibbonApp.cpp
    UI/Ribbon/RibbonSites.cpp
    UI/UIBehaviorClasses.cpp
    Preview/PreviewPresenter.cpp
    Preview/PreviewDataContext.cpp
    StoryboardManager/Serialization/SerializationContext.cpp
    StoryboardManager/Serialization/SerializationReader.cpp
    StoryboardManager/Serialization/SerializationWriter.cpp
    StoryboardManager/Serialization/SerializationMemoryReaderWriter.cpp
    StoryboardManager/Serialization/DatabasePropertyManager.cpp
    StoryboardManager/Serialization/ModElements.cpp
    StoryboardManager/Serialization/BoundPropertyDictionary.cpp
    StoryboardManager/Serialization/TextProperty.cpp
    StoryboardManager/Serialization/BoundPlaceholder.cpp
    StoryboardManager/Serialization/SnapShot.cpp
    StoryboardManager/Serialization/ValueVariantRef.cpp
    StoryboardManager/Serialization/SerializationClasses.cpp
    StoryboardManager/MediaItems/MediaItem.cpp
    StoryboardManager/Background/BackgroundOrchestrator.cpp
    StoryboardManager/Background/BackgroundRequests.cpp
    StoryboardManager/Background/AutofitProcess.cpp
    StoryboardManager/Background/Conductor.cpp
    StoryboardManager/Transport/TransportBase.cpp
    StoryboardManager/Transport/CommandBin.cpp
    HMREngine/AsyncWorker.cpp
    HMREngine/ErrHandler.cpp
    HMREngine/X3DNodes.cpp
    HMREngine/DXResources/EngineDX.cpp
    HMREngine/DXResources/TextureResourceDX.cpp
    HMREngine/DXResources/MeshResourceDX.cpp
    HMREngine/DXResources/EffectResourceDX.cpp
    HMREngine/DXResources/TextResourceDX.cpp
    HMREngine/DXResources/RenderLoopDX.cpp
    HMREngine/DXResources/ResourceCacheDX.cpp
    HMREngine/DXResources/TextureCodecs.cpp
    HMREngine/X3DNodeImpls/X3DNodeImplBase.cpp
    HMREngine/X3DNodeImpls/X3DGroupingImpls.cpp
    HMREngine/X3DNodeImpls/X3DAppearanceImpls.cpp
    HMREngine/X3DNodeImpls/X3DLayerImpls.cpp
    HMREngine/X3DNodeImpls/X3DSensorImpls.cpp
    HMREngine/PatternMesh/PatternMesh.cpp
    HMREngine/PatternMesh/PatternMeshFactory.cpp
    HMREngine/TextRender/TextRenderPipeline.cpp
    HMREngine/d3dx11compat.cpp
    HMRAVSource/dxva2stubs.cpp
    Publishing/PublishDialogs.cpp
    Publishing/PublishItem.cpp
    Publishing/PublishJob.cpp
    Publishing/PublishClasses.cpp
    Publishing/XmlManifest.cpp
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
    WMMR_DLL_EXPORTS
    _WINDLL
    _AFXDLL
    _CRT_SECURE_NO_WARNINGS
    _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
    _ATL_ALL_WARNINGS
)

target_include_directories(${TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/WTL
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
    d3dcompiler
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
    wininet
    WLXPhotoBase
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
