cmake_minimum_required(VERSION 3.10)
project(pixcii)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp")
add_executable(pixcii ${SOURCES})