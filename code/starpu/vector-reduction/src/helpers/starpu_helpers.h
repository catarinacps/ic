#pragma once

#include <starpu.h>
#include <stdlib.h>

#include "utils/utils.h"

typedef struct starpu_data_filter starpu_data_filter_t;

llint* alloc_and_register_integer_vector(starpu_data_handle_t* handle, ullint size);

llint* alloc_and_register_integer_variable(starpu_data_handle_t* handle);

void register_integer_variable(starpu_data_handle_t* handle, llint* variable);

void register_integer_vector(starpu_data_handle_t* handle, llint vector[], size_t size);

void partition_vector_handle(starpu_data_handle_t* handle, const uint n_parts);
