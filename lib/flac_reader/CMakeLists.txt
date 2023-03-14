cmake_minimum_required(VERSION 3.5)

project(reader:3) 
add_executable(reader main.cpp FLAC_track_info.cpp)

find_package(Flac REQUIRED)
target_link_libraries(reader PRIVATE FLAC::FLAC++)

