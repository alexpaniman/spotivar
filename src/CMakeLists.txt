add_executable(spotivar spotivar.cpp)

target_include_directories(spotivar PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(spotivar fmt axp spotivar-gtk-ui)
