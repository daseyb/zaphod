OPTION(EMBREE_STATIC_LIB "Build Embree as a static library." ON)
OPTION(EMBREE_ISPC_SUPPORT "Build Embree with support for ISPC applications." OFF)
OPTION(EMBREE_TUTORIALS    "Enable to build Embree tutorials" OFF)


set(TASKING_INTERNAL ON FORCE)
set(TASKING_TBB OFF FORCE)
set(EMBREE_TASKING_SYSTEM "INTERNAL" CACHE STRING "Selects tasking system" FORCE)
  
add_subdirectory(embree)
set(INC_DIRS ${INC_DIRS} "${CMAKE_CURRENT_LIST_DIR}/embree/include")
set(LIBS ${LIBS} "embree" "embree_sse42")

if(NOT HEADLESS)
  set(BUILD_SHARED_LIBS OFF CACHE STRING "" FORCE)
  option(SFML_USE_STATIC_STD_LIBS "" ON)
  add_subdirectory(SFML)
  set(INC_DIRS ${INC_DIRS} "${CMAKE_CURRENT_LIST_DIR}/SFML/include" PARENT_SCOPE)
  set(LIBS ${LIBS} "sfml-graphics" "sfml-window" "sfml-system" "sfml-main" PARENT_SCOPE)
endif()

# add_subdirectory(USD)
