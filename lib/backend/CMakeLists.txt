#cmake ../ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja
find_package(Boost COMPONENTS system filesystem REQUIRED)

find_package(Flac REQUIRED)

find_package(jsoncpp REQUIRED)

add_library(backend STATIC src/backend.cpp src/session.cpp)
target_include_directories(backend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(backend PUBLIC audio_data directory_scaner)

add_subdirectory(gtk)



