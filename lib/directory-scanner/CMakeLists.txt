# find_package(Boost COMPONENTS system filesystem REQUIRED)

add_library(directory-scanner STATIC src/user_files.cpp)
target_include_directories(directory-scanner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(directory-scanner PUBLIC audio-data)

