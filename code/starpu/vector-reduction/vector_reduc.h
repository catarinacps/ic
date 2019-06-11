#include <starpu.h>

void merge_sum(void** buffers, void* cl_arg);

void reduc_sum(void** buffers, void* cl_arg);

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
