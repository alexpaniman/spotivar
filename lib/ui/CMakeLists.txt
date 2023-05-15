project(ui) 
add_executable(ui backend.cpp main.cpp session.cpp ../audio_data_decoder/audio_data_reader.cpp)

add_library(spotivar-ui INTERFACE)
target_include_directories(spotivar-ui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Boost COMPONENTS system filesystem REQUIRED)
target_link_libraries(ui PRIVATE Boost::headers Boost::system Boost::filesystem)

find_package(Flac REQUIRED)
target_link_libraries(ui PRIVATE FLAC::FLAC++)

find_package(jsoncpp REQUIRED)
target_link_libraries(ui PUBLIC jsoncpp_lib)

add_subdirectory(gtk)