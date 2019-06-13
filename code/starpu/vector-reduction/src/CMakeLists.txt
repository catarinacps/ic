file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

link_directories(${STARPU_LIBRARY_DIRS})

add_executable(vector_reduc ${SOURCES})
target_link_libraries(vector_reduc ${STARPU_LIBRARIES_DEP})
