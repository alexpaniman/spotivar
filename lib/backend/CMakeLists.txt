add_library(backend STATIC src/backend.cpp src/session.cpp)
target_include_directories(backend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(backend PUBLIC audio-data directory-scanner spotivar-ui)

