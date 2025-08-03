cmake_minimum_required(VERSION 3.10)
project(pixcii)

# Set C++ standard to 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})

# Collect source files
file(GLOB SOURCES "src/*.cpp")

# Create executable
add_executable(pixcii ${SOURCES})

# Link libraries
target_link_libraries(pixcii ${OpenCV_LIBS})

# Link filesystem library for C++17
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(pixcii stdc++fs)
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(pixcii PRIVATE 
    -Wall -Wextra -O2
    -Wno-missing-field-initializers  # Suppress STB warnings
)

endif()