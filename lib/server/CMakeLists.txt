add_library(server STATIC server.cpp)

target_include_directories(server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
