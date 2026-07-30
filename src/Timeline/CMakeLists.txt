add_library(TimelineEngine STATIC
    Clip.h
    TimelineEngine.h
    TimelineEngine.cpp
    TimelineDispatcher.h
    TimelineDispatcher.cpp
)
target_include_directories(TimelineEngine PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/Playback
    ${CMAKE_SOURCE_DIR}/src/Media
)
