add_executable(spotivar spotivar.cpp)

set_target_properties(spotivar PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

target_include_directories(spotivar PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(spotivar fmt axp spotivar-gtk-ui quick-backend)
