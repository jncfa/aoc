set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CXX_STANDARD_REQUIRED ON)

#add_compile_options(-g3 -O0)
#add_link_options(-fsanitize=address)
set(WARNINGS_AS_ERRORS ON)
include(cmake/cpp_warnings.cmake)

add_executable(day11_p1 p1.cpp)
add_executable(day11_p2 p2.cpp)

target_compile_options(day11_p1 PRIVATE ${PROJECT_WARNING_FLAGS})
target_compile_options(day11_p2 PRIVATE ${PROJECT_WARNING_FLAGS})