# version required
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -g")

# configure information
project(hb_m_sdk LANGUAGES C)

add_subdirectory("app")
add_subdirectory("hal")
add_subdirectory("sdk")
add_subdirectory("utils")