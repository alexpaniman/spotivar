add_library(spotivar-ui INTERFACE)
target_include_directories(spotivar-ui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(gtk)
