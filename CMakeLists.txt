cmake_minimum_required (VERSION 3.1)
project (zaphod)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

option(HEADLESS "Build the renderer in headless mode" OFF)

if(HEADLESS)
    add_definitions(-DHEADLESS)
endif()

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(LIBS "")
set(INCLUDE "")

add_subdirectory(extern)

include_directories(${INCLUDE})

add_subdirectory(src)

