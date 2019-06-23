# Looks for source files in the directory this CMakeLists.txt is
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

link_directories(${STARPU_LIBRARY_DIRS})

add_executable(vector_reduc ${SOURCES})
target_link_libraries(vector_reduc ${STARPU_LIBRARIES_DEP})
set_property(TARGET vector_reduc PROPERTY C_STANDARD 11)
