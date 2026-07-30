set(TARGET GPURenderer)

add_library(${TARGET} SHARED
    Renderer.h
    Renderer.cpp
)

set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME "GPURenderer"
    PREFIX ""
    SUFFIX ".dll"
    WIN32_EXECUTABLE NO
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
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
    ole32
    d3d11
    d2d1
    dxgi
)

install(TARGETS ${TARGET}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
