# ==> Ensure proper CMake support

cmake_minimum_required(VERSION 3.10)

# ==> Force new C++ standard (we like new features here)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(spotivar VERSION 1.0)

# ==> Build in release by default (makes difference for graphical projects)

set(DEFAULT_BUILD_TYPE "Release")

# ==> Add option to force ANSI-colors (useful for building with Ninja)

option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." FALSE)

if (${FORCE_COLORED_OUTPUT})
  if     ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options (-fdiagnostics-color=always)
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options (-fcolor-diagnostics)
  endif ()
endif ()

# ==> Enable as much optimization as possible (note, some of these are dangerous)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D NDEBUG -O2")

# ==> Flags for debugging, enable sanitizers and make debugging experience as good as possible

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D _DEBUG -ggdb3 -std=c++20 -Wall -Wextra -Weffc++ -Wcast-align -Wcast-qual -Wchar-subscripts -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat=2 -Winline -Wnon-virtual-dtor -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls -Wsign-promo -Wstrict-overflow=2 -Wsuggest-override -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused -Wvariadic-macros -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -fcheck-new -fsized-deallocation -fstack-check -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -fPIE -march=native -mtune=native -O0")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Waggressive-loop-optimizations -Wconditionally-supported -Wformat-signedness -Wlogical-op -Wopenmp-simd -Wstrict-null-sentinel -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsync-nand -Wuseless-cast -fconcepts-diagnostics-depth=3 -Wno-literal-suffix")
endif ()

# ==> Add libraries

add_subdirectory(lib/server)
add_subdirectory(lib/search)
add_subdirectory(lib/fmt)
add_subdirectory(lib/axp)
add_subdirectory(lib/sha256)

# ==> Add project's core 

add_subdirectory(src)
