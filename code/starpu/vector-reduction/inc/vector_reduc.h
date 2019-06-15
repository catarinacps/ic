#include <math.h>
#include <starpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "starpu_helpers.h"

void merge_sum(void** buffers, void* cl_arg);

void reduc_sum(void** buffers, void* cl_arg);

unsigned int merge_depth(unsigned int items, unsigned int degree);

int submit_reduction_task(int blksize, int blkid, starpu_data_handle_t* input_handle, starpu_data_handle_t* output_handle);

double get_time(void);

struct starpu_codelet reduc_cl = {
    .where = STARPU_CPU,
    .cpu_funcs = { reduc_sum },
    .nbuffers = 2,
    .modes = { STARPU_R, STARPU_W }
};

struct starpu_codelet merge_cl = {
    .where = STARPU_CPU,
    .cpu_funcs = { merge_sum }
};

typedef struct {
    unsigned int nbuffers;
} merge_params_t;

typedef struct {
    unsigned int begin;
    unsigned int end;
} reduc_params_t;
