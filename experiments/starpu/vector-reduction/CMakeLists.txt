cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

project(vector_reduction LANGUAGES CXX)

include_directories(include)

file(GLOB_RECURSE SOURCES RELATIVE ${PROJECT_SOURCE_DIR} "src/*.cpp")

find_package(STARPU REQUIRED)
include_directories(${STARPU_INCLUDE_DIRS})
link_directories(${STARPU_LIBRARY_DIRS})

add_executable(main ${SOURCES})
target_link_libraries(main ${STARPU_LIBRARIES_DEP})
