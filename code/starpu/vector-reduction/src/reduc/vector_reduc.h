#pragma once

#include <starpu.h>

#include "helpers/starpu_helpers.h"
#include "utils/utils.h"

#define DEFAULT_INITIAL_VALUE 2

void reduc_sum(void** buffers, void* cl_arg);

int submit_reduction_task(starpu_data_handle_t* input_handle, starpu_data_handle_t* output_handle);
