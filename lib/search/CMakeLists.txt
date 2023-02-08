add_library(search STATIC regex.cpp)

target_include_directories(search PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
