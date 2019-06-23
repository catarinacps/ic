#pragma once

#include <starpu.h>

typedef struct starpu_data_filter starpu_data_filter_t;

int* alloc_and_register_integer_vector(starpu_data_handle_t* handle, size_t size);

int* alloc_and_register_integer_variable(starpu_data_handle_t* handle);

void register_integer_variable(starpu_data_handle_t* handle, int* variable);

void register_integer_vector(starpu_data_handle_t* handle, int vector[], size_t size);

void partition_vector_handle(starpu_data_handle_t* handle, const unsigned int n_parts);
