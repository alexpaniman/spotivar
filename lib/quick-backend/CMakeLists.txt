add_library(quick-backend STATIC src/quick-backend.cpp)
target_include_directories(quick-backend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(quick-backend PUBLIC audio-data spotivar-ui client)

