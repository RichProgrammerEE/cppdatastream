# cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install -G Ninja
# cmake --build build --config Release --clean-first
cmake_minimum_required(VERSION 3.22)

# Define the project
set(MAIN_TARGET datastream)
project(${MAIN_TARGET} VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Always link the multi-threaded C runtime on windows as all our dependencies are doing this as well.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

include(GNUInstallDirs)

# External dependencies
include(deps/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Define target
add_executable(${MAIN_TARGET}
    src/Main.cpp
    src/Utilities.cpp)

target_include_directories(${MAIN_TARGET} PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

target_link_libraries(${MAIN_TARGET} PRIVATE 
    CONAN_PKG::fmt CONAN_PKG::boost)

# Include some platform specific optimization
if(MSVC)
    target_compile_options(${MAIN_TARGET} PRIVATE -W4 -WX /arch:AVX2 $<$<CONFIG:Release>:/GL>)
    target_link_options(${MAIN_TARGET} PRIVATE /arch:AVX2 $<$<CONFIG:Release>:/LTCG:STATUS>)
else()
    target_compile_options(${MAIN_TARGET} PRIVATE -Wall -Wextra -pedantic -Werror -Wsuggest-override -march=native)
    target_link_options(${MAIN_TARGET} PRIVATE -march=native)
endif()