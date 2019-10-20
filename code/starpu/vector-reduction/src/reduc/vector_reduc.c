#include "vector_reduc.h"

void reduc_sum(void** buffers, void* cl_arg)
{
    ullint* vec_input = (ullint*)STARPU_VECTOR_GET_PTR(buffers[0]);
    ullint* output = (ullint*)STARPU_VARIABLE_GET_PTR(buffers[1]);
    unsigned nx_input = STARPU_VECTOR_GET_NX(buffers[0]);

    double t0 = get_time();

    // do the job
    for (uint i = 0; i < nx_input; i++)
        *output += vec_input[i];

    double t1 = get_time();

    V_PRINTF("SUM = %d\n"
             "Task finished work with elapsed time %f\n",
        *output, t1 - t0);
}

int submit_reduction_task(starpu_data_handle_t* input_handle, starpu_data_handle_t* output_handle)
{
    struct starpu_task* task = starpu_task_create();

    task->synchronous = 0;
    task->cl = &reduc_cl;
    task->handles[0] = *input_handle;
    task->handles[1] = *output_handle;

    return starpu_task_submit(task);
}
