#include <math.h>
#include <starpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>

#include "starpu_helpers.h"

#define MAX_RAND 2048
#define MIN_RAND -2048

#define INITIAL_VALUE 2

typedef unsigned int uint;

void reduc_sum(void** buffers, void* cl_arg);

int generate_random_int(const int max, const int min);

int submit_reduction_task(starpu_data_handle_t* input_handle, starpu_data_handle_t* output_handle);

double get_time(void);

struct starpu_codelet reduc_cl = {
    .where = STARPU_CPU,
    .cpu_funcs = { reduc_sum },
    .nbuffers = 2,
    .modes = { STARPU_R, STARPU_W }
};
