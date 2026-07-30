add_library(TimelineEngine STATIC
    Clip.h
    TimelineEngine.h
    TimelineEngine.cpp
)
target_include_directories(TimelineEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
