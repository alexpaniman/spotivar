add_library(axp STATIC src/exceptions.cpp src/strings.cpp src/source-location.cpp)

target_include_directories(axp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(axp PUBLIC fmt)
