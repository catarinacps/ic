#include <math.h>
#include <starpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>

#include "starpu_helpers.h"

#ifdef VERBOSE_ENABLED
#define V_PRINTF(f_, ...) printf((f_), ##__VA_ARGS__)
#else
#define V_PRINTF(f_, ...) ((void)0)
#endif

#define INITIAL_VALUE 2

typedef unsigned int uint;
typedef unsigned long long ullint;

void reduc_sum(void** buffers, void* cl_arg);

int generate_random_int(const int max, const int min);

int submit_reduction_task(starpu_data_handle_t* input_handle, starpu_data_handle_t* output_handle);

double get_time(void);

struct starpu_codelet reduc_cl = {
    .name = "Reduction",
    .where = STARPU_CPU,
    .cpu_funcs = { reduc_sum },
    .nbuffers = 2,
    .modes = { STARPU_R, STARPU_W }
};
