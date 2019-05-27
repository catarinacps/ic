cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

project(vector_reduction LANGUAGES CXX)

add_subdirectory(starpu-cpp/)

include_directories(inc/)

file(GLOB_RECURSE SOURCES RELATIVE ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(main ${SOURCES})
target_link_libraries(main starpu-cpp)
