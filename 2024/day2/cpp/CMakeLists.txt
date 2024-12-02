project(day2)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

#add_compile_options(-g3 -O0)
#add_link_options(-fsanitize=address)

add_executable(p1 p1.cpp)
add_executable(p2 p2.cpp)