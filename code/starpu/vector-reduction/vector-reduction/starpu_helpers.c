#include "starpu_helpers.h"

int* alloc_and_register_integer_vector(starpu_data_handle_t* handle, size_t size)
{
    int* vector = (int*)malloc(size * sizeof(int));

    memset(vector, 0, size * sizeof(int));

    starpu_vector_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)vector, size, sizeof(int));

    return vector;
}

int* alloc_and_register_integer_variable(starpu_data_handle_t* handle)
{
    int* data = (int*)malloc(sizeof(int));

    starpu_variable_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)data, sizeof(int));

    return data;
}

void register_integer_variable(starpu_data_handle_t* handle, int* variable)
{
    starpu_variable_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)variable, sizeof(int));
}

void register_integer_vector(starpu_data_handle_t* handle, int vector[], size_t size)
{
    starpu_vector_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)vector, size, sizeof(int));
}

void partition_vector_handle(starpu_data_handle_t* handle, const unsigned int n_parts)
{
    starpu_data_filter_t f = { .filter_func = starpu_vector_filter_block, .nchildren = n_parts };

    starpu_data_partition(*handle, &f);
}
