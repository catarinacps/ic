cmake_minimum_required (VERSION 3.10)

# This build depends on spack to have StarPU installed
# So, make sure it's in PATH
execute_process(COMMAND spack location -i starpu OUTPUT_VARIABLE STARPU_PATH)

# Now we define the important build variables
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${STARPU_PATH}/lib/pkgconfig")
set(ENV{STARPU_DIR} "${STARPU_PATH}" CACHE PATH "The StarPU directory")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# The finder function is inside cmake/
# You may have to run the cmake invocation twice to actually find it
find_package(STARPU REQUIRED)

include_directories(${STARPU_INCLUDE_DIRS})

add_subdirectory(${CMAKE_SOURCE_DIR}/vector-reduction ${CMAKE_SOURCE_DIR}/bin)
