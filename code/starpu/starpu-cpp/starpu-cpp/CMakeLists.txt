find_package(STARPU REQUIRED)

include_directories(inc)
include_directories(${STARPU_INCLUDE_DIRS})

file(GLOB_RECURSE LIB_SOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

link_directories(${STARPU_LIBRARY_DIRS})

add_library(starpu-cpp ${LIB_SOURCES})

target_include_directories(starpu-cpp INTERFACE inc)
target_link_libraries(starpu-cpp ${STARPU_LIBRARIES_DEP})
