#include "vector_reduc.h"

double get_time(void)
{
    struct timeval tr;
    gettimeofday(&tr, NULL);
    return (double)tr.tv_sec + (double)tr.tv_usec / 1000000;
}

int generate_random_int(const int max, const int min)
{
    return rand() % (max + 1 - min) + min;
}

unsigned int merge_depth(unsigned int items, unsigned int degree)
{
    unsigned int ret = 1;

    while (items > pow(degree, ret)) {
        ret++;
    }

    return ret;
}

void reduc_sum(void** buffers, void* cl_arg)
{
    int* vec_input = (int*)STARPU_VECTOR_GET_PTR(buffers[0]);
    int* output = (int*)STARPU_VARIABLE_GET_PTR(buffers[1]);
    unsigned nx_input = STARPU_VECTOR_GET_NX(buffers[0]);

    reduc_params_t* par = (reduc_params_t*)cl_arg;

    double t0 = get_time();
    /* printf("%f Task started %p - %p, begin=%d, end=%d\n", */
    /* 	   t0, vec_input, vec_output, par->begin, par->end); */

    // do the job
    for (int i = 0; i < nx_input; i++) {
        *output += vec_input[i];
    }

    double t1 = get_time();

    printf("======> Sum = %d\n", *output);
    /* printf("%f Task finished to work with begin=%d (%f)\n", */
    /* 	   t1, par->begin, t1 - t0); */
}

struct starpu_task* submit_reduction_task(
    int blksize,
    int blkid,
    starpu_data_handle_t* input_handle,
    starpu_data_handle_t* output_handle,
    unsigned int n_deps,
    struct starpu_task* dependencies[])
{
    reduc_params_t* params;
    params = (reduc_params_t*)malloc(sizeof(reduc_params_t));

    // prepare the parameters
    params->begin = blksize * blkid;
    params->end = blksize * blkid + blksize;

    struct starpu_task* task = starpu_task_create();

    task->synchronous = 0;
    task->cl = &reduc_cl;
    task->cl_arg = params;
    task->cl_arg_size = sizeof(reduc_params_t);
    task->handles[0] = *input_handle;
    task->handles[1] = *output_handle;

    if (dependencies != NULL && n_deps > 0) {
        starpu_task_declare_deps_array(task, n_deps, dependencies);
    }

    int ec = starpu_task_submit(task);
    return task;
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        printf("Please provide the following parameters:\n"
               "%s <problem_size> <block_size>\n",
            argv[0]);
        exit(-1);
    }

    unsigned int n_elements = atoi(argv[1]);
    unsigned int original_n_elements = n_elements;
    // now the block_size
    unsigned int block_size = atoi(argv[2]);

    unsigned int n_merges = (int)ceil((float)n_elements / block_size);
    const unsigned int depth = merge_depth(n_elements, block_size);

    printf("n_merges = %d\n", n_merges);
    printf("depth = %d\n", depth);

    /* const unsigned int degree = atoi(argv[3]); */

    if (block_size <= 1) {
        printf("Please insert a block size bigger than 1\n");
        exit(-1);
    }

    // calculating the block_size
    printf("There are %d blocks, each one with %d elements.\n", n_merges, block_size);

    int ec = starpu_init(NULL);
    starpu_profiling_status_set(STARPU_PROFILING_DISABLE);

    double ts0 = get_time();

    // INPUT
    int* input_vector = (int*)malloc(sizeof(int) * (n_elements + block_size));
    for (int i = 0; i < n_elements; i++)
        input_vector[i] = 2;
    // setting the initial value with memset is a cool trick that only works
    // if you want to init it to 0!!
    memset(&input_vector[n_elements], 0, block_size * sizeof(int));

    starpu_data_handle_t* input_handles;

    // OUTPUT
    int* output_vector;

    struct starpu_task** tasks_vector;
    struct starpu_task** last_tasks_vector;
    unsigned int n_deps = 0;

    for (int i = 0; i < depth; i++) {
        starpu_data_handle_t* output_handles = malloc(sizeof(starpu_data_handle_t) * n_merges);

        input_handles = malloc(sizeof(starpu_data_handle_t) * n_merges);

        output_vector = malloc(sizeof(int) * (n_merges + block_size));
        memset(output_vector, 0, (n_merges + block_size) * sizeof(int));

        tasks_vector = malloc(sizeof(struct starpu_task*) * n_merges);

        for (int k = 0, j = 0; j < n_elements; j += block_size, k++) {
            register_integer_vector(&input_handles[k], &input_vector[j], block_size);

            register_integer_variable(&output_handles[k], &output_vector[k]);

            tasks_vector[k] = submit_reduction_task(
                block_size,
                k,
                &input_handles[k],
                &output_handles[k],
                /* original_n_elements == n_elements ? 0 : block_size, */
                /* original_n_elements == n_elements ? NULL : &last_tasks_vector[k]); */
                0,
                NULL);
        }

        //replace inputs vector by the smaller output vector
        input_vector = output_vector;
        n_elements = n_merges;
        last_tasks_vector = tasks_vector;
        n_merges = (int)ceil((float)n_elements / block_size);
    }

    starpu_task_wait_for_all();
    //  starpu_data_unregister(vec_handle);
    //  starpu_data_unregister(vec_output_handle);
    starpu_shutdown();

    double ts1 = get_time();
    double elapsed = ts1 - ts0;
    printf("start: %.4f\nend: %.4f\nelapsed: %.4f\n", ts0, ts1, elapsed);
}
