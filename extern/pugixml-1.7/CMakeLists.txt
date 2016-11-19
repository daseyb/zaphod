project(PugiXML)

set(PUGI_INCLUDE_DIRS src)

include_directories(${PUGI_INCLUDE_DIRS})

add_library(PugiXML "src/pugixml.cpp")



