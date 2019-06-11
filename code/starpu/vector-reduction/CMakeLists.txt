cmake_minimum_required (VERSION 2.8)

execute_process(COMMAND spack location -i starpu OUTPUT_VARIABLE OUT_DIR)
set(STARPU_DIR ${OUT_DIR} CACHE PATH "The StarPU directory")
set(ENV{STARPU_DIR} ${OUT_DIR})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(STARPU REQUIRED)
include_directories(${STARPU_INCLUDE_DIRS})
link_directories(${STARPU_LIBRARY_DIRS})

add_executable(vector_reduc vector_reduc.c)
target_link_libraries(vector_reduc ${STARPU_LIBRARIES_DEP})
