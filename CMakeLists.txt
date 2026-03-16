cmake_minimum_required(VERSION 3.16)

project(ArcFlags LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

file(GLOB SRCS CONFIGURE_DEPENDS "*.cpp")

add_executable(arcflag ${SRCS})
target_include_directories(arcflag PUBLIC datastructures helpers)

target_compile_options(arcflag PRIVATE
    -Wall
    -Wextra
    -march=native
)

find_package(hwy REQUIRED)

target_link_libraries(arcflag PRIVATE hwy)
target_link_libraries(arcflag PRIVATE pthread)

target_compile_options(arcflag PRIVATE
    $<$<CONFIG:Debug>:-O0 -g>
)

target_compile_options(arcflag PRIVATE
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)
