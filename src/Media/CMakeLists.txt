cmake_minimum_required(VERSION 3.15)

add_library(MediaCatalog STATIC
    MediaCatalog.cpp
)

set_target_properties(MediaCatalog PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
)

target_include_directories(MediaCatalog PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(MediaCatalog PUBLIC
    mfplat
    mfreadwrite
    mf
)
