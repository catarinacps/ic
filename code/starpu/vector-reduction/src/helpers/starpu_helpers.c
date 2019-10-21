#include "starpu_helpers.h"

llint* alloc_and_register_integer_vector(starpu_data_handle_t* handle, ullint size)
{
    llint* vector = (llint*)malloc(size * sizeof(llint));

    if (vector == NULL)
        return NULL;

    memset(vector, 0, size * sizeof(llint));

    starpu_vector_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)vector, size, sizeof(llint));

    return vector;
}

llint* alloc_and_register_integer_variable(starpu_data_handle_t* handle)
{
    llint* data = (llint*)malloc(sizeof(llint));

    if (data == NULL)
        return NULL;

    starpu_variable_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)data, sizeof(llint));

    return data;
}

void register_integer_variable(starpu_data_handle_t* handle, llint* variable)
{
    starpu_variable_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)variable, sizeof(llint));
}

void register_integer_vector(starpu_data_handle_t* handle, llint vector[], size_t size)
{
    starpu_vector_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)vector, size, sizeof(llint));
}

void partition_vector_handle(starpu_data_handle_t* handle, const uint n_parts)
{
    starpu_data_filter_t f = { .filter_func = starpu_vector_filter_block, .nchildren = n_parts };

    starpu_data_partition(*handle, &f);
}
