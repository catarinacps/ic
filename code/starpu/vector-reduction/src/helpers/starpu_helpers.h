#pragma once

#include <starpu.h>
#include <stdlib.h>

#include "utils/utils.h"

typedef struct starpu_data_filter starpu_data_filter_t;

ullint* alloc_and_register_integer_vector(starpu_data_handle_t* handle, size_t size);

ullint* alloc_and_register_integer_variable(starpu_data_handle_t* handle);

void register_integer_variable(starpu_data_handle_t* handle, ullint* variable);

void register_integer_vector(starpu_data_handle_t* handle, ullint vector[], size_t size);

void partition_vector_handle(starpu_data_handle_t* handle, const uint n_parts);
