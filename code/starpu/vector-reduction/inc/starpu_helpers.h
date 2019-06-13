#pragma once

#include <starpu.h>

void alloc_and_register_integer_vector(starpu_data_handle_t* handle, size_t size, int init_value);
