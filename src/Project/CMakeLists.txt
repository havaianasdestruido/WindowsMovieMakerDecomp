cmake_minimum_required(VERSION 3.15)
project(ProjectManager LANGUAGES CXX)
add_library(ProjectManager STATIC Project.cpp)
target_include_directories(ProjectManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
