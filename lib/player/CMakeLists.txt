cmake_minimum_required(VERSION 3.25)

project(varvarvaravar) # TODO: rename

add_executable(player main.cpp buffer.cpp device.cpp music_buffer.cpp source.cpp)

find_package(OpenAL CONFIG REQUIRED)
target_link_libraries(player PRIVATE OpenAL::OpenAL)

find_package(SndFile REQUIRED)
target_link_libraries(player PRIVATE SndFile::sndfile)

find_package(Flac REQUIRED)
target_link_libraries(player PRIVATE FLAC::FLAC++)


#cmake ../ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja
