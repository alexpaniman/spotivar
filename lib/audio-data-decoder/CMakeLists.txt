add_library(audio-data STATIC src/audio_data_reader.cpp)

target_include_directories(audio-data PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(audio-data flac jsoncpp_lib)
