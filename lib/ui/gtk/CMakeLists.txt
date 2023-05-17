project(spotivar-gtk-ui)


# embed .glade layout in code

set(glade-compiled-file "${CMAKE_CURRENT_BINARY_DIR}/spotivar.glade.cpp")

add_custom_command(OUTPUT ${glade-compiled-file}
    COMMAND m4 -I ${CMAKE_CURRENT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/res/spotivar.glade.cpp.m4 >
	${glade-compiled-file}
    DEPENDS ${PROJECT_SOURCE_DIR}/res/spotivar.glade.cpp.m4
    DEPENDS ${PROJECT_SOURCE_DIR}/res/spotivar.glade
    COMMENT "Generating wrapper for glade ui" )

# make ui into library 

add_library(${PROJECT_NAME}
    src/spotivar-gtk-view.cpp
    ${glade-compiled-file})

target_include_directories(${PROJECT_NAME} PUBLIC include/)

# include gtkmm library

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0 REQUIRED)

target_link_directories(${PROJECT_NAME} PRIVATE ${GTKMM_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${GTKMM_LIBRARIES} spotivar-ui fmt audio-data)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${GTKMM_INCLUDE_DIRS})
