cmake_minimum_required (VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(STARPU)
include_directories(${STARPU_INCLUDE_DIRS})
link_directories(${STARPU_LIBRARY_DIRS})

add_executable(programa programa.c)
target_link_libraries(programa ${STARPU_LIBRARIES_DEP})

add_executable(vector_scal vector_scal.c)
target_link_libraries(vector_scal ${STARPU_LIBRARIES_DEP})

