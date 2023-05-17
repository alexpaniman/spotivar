add_library(server STATIC server.cpp connection.cpp)
add_library(client STATIC client.cpp connection.cpp)

find_package(Boost REQUIRED)

target_link_libraries(server PUBLIC ${Boost_LIBRARY_DIRS} axp fmt flac)
target_link_libraries(client PUBLIC ${Boost_LIBRARY_DIRS} axp fmt flac)

target_include_directories(server PUBLIC ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(client PUBLIC ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})