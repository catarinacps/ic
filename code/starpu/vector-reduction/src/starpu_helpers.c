#include "starpu_helpers.h"

void alloc_and_register_integer_vector(starpu_data_handle_t* handle, size_t size, int init_value = 0)
{
    int* vector = (int*)malloc(size * sizeof(int));

    for (int i; i < size; i++) {
        vector[i] = init_value;
    }

    starpu_vector_data_register(handle, STARPU_MAIN_RAM, (uintptr_t)vector, size, sizeof(int));
}
