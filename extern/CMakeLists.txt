set(ENABLE_STATIC_LIB ON CACHE BOOL "Build Embree as a static library." FORCE)
set(ENABLE_ISPC_SUPPORT OFF CACHE BOOL "Build Embree with support for ISPC applications." FORCE)
set(ENABLE_TUTORIALS  OFF CACHE BOOL "Enable to build Embree tutorials" FORCE)
SET(TASKING_INTERNAL ON CACHE BOOL "" FORCE)
SET(TASKING_TBB      OFF CACHE BOOL "" FORCE)
SET(RTCORE_TASKING_SYSTEM "INTERNAL" CACHE STRING "Selects tasking system" FORCE)
  
add_subdirectory(embree)
set(INC_DIRS ${INC_DIRS} "${CMAKE_CURRENT_LIST_DIR}/embree/include")
set(LIBS ${LIBS} "embree" "embree_sse42")

if(NOT HEADLESS)
  add_subdirectory(SFML)
  set(INC_DIRS ${INC_DIRS} "${CMAKE_CURRENT_LIST_DIR}/SFML/include" PARENT_SCOPE)
  set(LIBS ${LIBS} "sfml-graphics" "sfml-window" "sfml-system" "sfml-main" PARENT_SCOPE)
endif()

# add_subdirectory(USD)
