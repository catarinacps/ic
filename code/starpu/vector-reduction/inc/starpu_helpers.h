#pragma once

#include <starpu.h>

int* alloc_and_register_integer_vector(starpu_data_handle_t* handle, size_t size, int init_value);

int* alloc_and_register_integer_variable(starpu_data_handle_t* handle, int init_value);

void register_integer_variable(starpu_data_handle_t* handle, int* variable);

void register_integer_vector(starpu_data_handle_t* handle, int vector[], size_t size);
