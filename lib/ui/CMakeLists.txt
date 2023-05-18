add_library(spotivar-ui INTERFACE)
target_include_directories(spotivar-ui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(spotivar-ui INTERFACE backend audio-data directory-scanner)

add_subdirectory(gtk)
