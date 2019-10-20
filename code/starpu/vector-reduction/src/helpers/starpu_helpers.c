#include "starpu_helpers.h"

ullint* alloc_and_register_integer_vector(starpu_data_handle_t* handle, ullint size)
{
    ullint* vector = (ullint*)malloc(size * sizeof(ullint));

    if (vector == NULL)
        return NULL;

    memset(vector, 0, size * sizeof(ullint);

    starpu_vector_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)vector, size, sizeof(ullint));

    return vector;
}

ullint* alloc_and_register_integer_variable(starpu_data_handle_t* handle)
{
    ullint* data = (ullint*)malloc(sizeof(ullint));

    if (data == NULL)
        return NULL;

    starpu_variable_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)data, sizeof(ullint));

    return data;
}

void register_integer_variable(starpu_data_handle_t* handle, ullint* variable)
{
    starpu_variable_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)variable, sizeof(ullint));
}

void register_integer_vector(starpu_data_handle_t* handle, ullint vector[], size_t size)
{
    starpu_vector_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)vector, size, sizeof(ullint));
}

void partition_vector_handle(starpu_data_handle_t* handle, const uint n_parts)
{
    starpu_data_filter_t f = { .filter_func = starpu_vector_filter_block, .nchildren = n_parts };

    starpu_data_partition(*handle, &f);
}
