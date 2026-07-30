cmake_minimum_required(VERSION 3.15)
project(ProjectManager LANGUAGES CXX)
add_library(ProjectManager STATIC Project.cpp)
target_include_directories(ProjectManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# tinyxml2 header only – fetch from internet
include(FetchContent)
FetchContent_Declare(
    tinyxml2
    URL https://raw.githubusercontent.com/leethomason/tinyxml2/master/tinyxml2.h
)
FetchContent_MakeAvailable(tinyxml2)

target_sources(ProjectManager PRIVATE ${tinyxml2_SOURCE_DIR}/tinyxml2.h)
