cmake_minimum_required(VERSION 3.22)

project(qrhi-tutorials VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Default build type: Release." FORCE)
endif ()

find_package(Qt6 6.8 COMPONENTS Core Gui Widgets ShaderTools REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (WIN32)
    # GUI/console
   set(CMAKE_WIN32_EXECUTABLE $<IF:$<CONFIG:DEBUG>,OFF,ON>)
endif ()

add_subdirectory(tutorial-01)
add_subdirectory(tutorial-02)
add_subdirectory(tutorial-03)