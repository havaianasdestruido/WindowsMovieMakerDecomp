cmake_minimum_required(VERSION 3.15)
project(WMMRTest LANGUAGES CXX)
add_executable(wmmr_test main.cpp)
# Assume binary output dir for MovieMakerCore.dll is build\bin\Debug
# Adjust path as needed for your configuration
set(DLL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../build/bin/Debug")
if(EXISTS "${DLL_DIR}/MovieMakerCore.dll")
    target_link_directories(wmmr_test PRIVATE "${DLL_DIR}")
    target_link_libraries(wmmr_test PRIVATE "MovieMakerCore")
else()
    message(FATAL_ERROR "MovieMakerCore.dll not found at ${DLL_DIR}")
endif()