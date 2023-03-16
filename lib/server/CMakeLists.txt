#add_library(server STATIC client.cpp)

find_package(Boost REQUIRED)

add_executable(server server.cpp)
add_executable(client client.cpp)

target_link_libraries(server PUBLIC ${Boost_LIBRARY_DIRS} axp fmt flac)
target_link_libraries(client PUBLIC ${Boost_LIBRARY_DIRS} axp fmt flac)

target_include_directories(server PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(client PUBLIC ${Boost_INCLUDE_DIRS})
