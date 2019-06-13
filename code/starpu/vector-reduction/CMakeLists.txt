cmake_minimum_required (VERSION 3.10)

# This build depends on spack to have StarPU installed
# So, make sure it's in PATH
execute_process(COMMAND spack location -i starpu OUTPUT_VARIABLE OUT_DIR)

# Now we define the important build variables
set(STARPU_DIR ${OUT_DIR} CACHE PATH "The StarPU directory")
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# The finder function is inside cmake/
find_package(STARPU REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/inc ${STARPU_INCLUDE_DIRS})

add_subdirectory(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/bin)
