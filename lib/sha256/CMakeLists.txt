add_library(sha256 STATIC src/sha256.cpp)

target_include_directories(sha256 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
