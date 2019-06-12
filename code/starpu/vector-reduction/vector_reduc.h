#include <math.h>
#include <starpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void alloc_and_register_integer_vector(starpu_data_handle_t* handle, size_t size, int init_value);

void merge_sum(void** buffers, void* cl_arg);

void reduc_sum(void** buffers, void* cl_arg);

unsigned int merge_depth(unsigned int items, unsigned int degree);

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
