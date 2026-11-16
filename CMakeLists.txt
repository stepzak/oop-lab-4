cmake_minimum_required(VERSION 3.20)
project(oop_lab_4)

set(CMAKE_CXX_STANDARD 20)

add_executable(oop_lab_4 main.cpp
        include/polygons.hpp
        include/small_vec.hpp
        src/polygons.cpp)

target_include_directories(oop_lab_4 PUBLIC include)