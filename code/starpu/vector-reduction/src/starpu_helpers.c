#include "starpu_helpers.h"

int* alloc_and_register_integer_vector(starpu_data_handle_t* handle, size_t size, int init_value)
{
    int* vector = (int*)malloc(size * sizeof(int));

    for (int i; i < size; i++) {
        vector[i] = init_value;
    }

    starpu_vector_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)vector, size, sizeof(int));

    return vector;
}

int* alloc_and_register_integer_variable(starpu_data_handle_t* handle, int init_value)
{
    int* data = (int*)malloc(sizeof(int));

    *data = init_value;

    starpu_variable_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)data, sizeof(int));

    return data;
}
