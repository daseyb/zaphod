cmake_minimum_required (VERSION 3.1)
project (zaphod C CXX)

# We need C++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

include_directories(${CMAKE_SOURCE_DIR}/Zaphod)

file(GLOB_RECURSE SRC
    "*.h"
    "*.cpp"
)

add_executable(zaphod_exe ${SRC})
SET_TARGET_PROPERTIES ( zaphod_exe PROPERTIES OUTPUT_NAME zaphod)