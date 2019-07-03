#include "vector_reduc.h"

int debug = 0;

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

void reduc_sum(void** buffers, void* cl_arg)
{
    int* vec_input = (int*)STARPU_VECTOR_GET_PTR(buffers[0]);
    int* output = (int*)STARPU_VARIABLE_GET_PTR(buffers[1]);
    unsigned nx_input = STARPU_VECTOR_GET_NX(buffers[0]);

    double t0 = get_time();
    /* printf("%f Task started %p - %p, begin=%d, end=%d\n", */
    /* 	   t0, vec_input, vec_output, par->begin, par->end); */

    // do the job
    for (int i = 0; i < nx_input; i++)
        *output += vec_input[i];

    double t1 = get_time();

    printf("======> Sum = %d\n", *output);
    /* printf("%f Task finished to work with begin=%d (%f)\n", */
    /* 	   t1, par->begin, t1 - t0); */
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

int main(int argc, char** argv)
{
    if (argc != 4) {
        printf("Please provide the following parameters:\n"
               "%s <problem_size> <number_blocks> <decay_factor>\n",
            argv[0]);
        exit(-1);
    }

    //--------------------------------------------------------------------------
    // cli arguments
    //--------------------------------------------------------------------------

    const long long original_n_elements = atoll(argv[1]);
    long long n_blocks = atoi(argv[2]);
    const long long decay_factor = atoi(argv[3]);

    long long block_size = (long long)ceil((double)original_n_elements / n_blocks);

    printf("number of blocks = %d\n", n_blocks);

    long long n_elements = n_blocks * block_size;

    if (n_blocks < 1 || n_blocks > n_elements) {
        printf("Please provide a number of blocks bigger than 0 and smaller than the number of elements\n");
        exit(-1);
    }

    if (decay_factor > n_blocks || decay_factor <= 1) {
        printf("the decay factor must be smaller or equal to the number of blocks and bigger than 1\n");
        exit(-1);
    }

    printf("There are %d blocks, each one with %d elements.\n", n_blocks, block_size);

    int ec = starpu_init(NULL);
    starpu_profiling_status_set(STARPU_PROFILING_DISABLE);

    double ts0 = get_time();

    //--------------------------------------------------------------------------
    // the algorithm actually starts here
    //--------------------------------------------------------------------------

    // INPUT
    starpu_data_handle_t input_handle;
    int* input_vector = alloc_and_register_integer_vector(&input_handle, n_elements);

    // initialize the vector
    for (int i = 0; i < original_n_elements; i++)
        input_vector[i] = INITIAL_VALUE;

    // OUTPUT
    starpu_data_handle_t output_handle;

    partition_vector_handle(&input_handle, n_blocks);

    uint depth = 0;
    bool not_top_level = false;
    while (n_blocks >= 1 && n_elements > 1) {
        printf("depth = %d\n"
               "block size = %d\n"
               "number of blocks = %d\n"
               "number of elements = %d\n",
            depth++, block_size, n_blocks, n_elements);

        n_elements = n_blocks;
        n_blocks = (long long)ceil((double)n_blocks / decay_factor);
        block_size = (long long)ceil((double)n_elements / n_blocks);
        n_elements = n_blocks * block_size;

        // here we don't need the actual vector as we won't be initializing it
        // to any arbitrary value
        alloc_and_register_integer_vector(&output_handle, n_elements);

        starpu_data_filter_t f_b = { .filter_func = starpu_vector_filter_block, .nchildren = n_blocks };
        starpu_data_filter_t f_e = { .filter_func = starpu_vector_filter_block, .nchildren = block_size };

        starpu_data_map_filters(output_handle, 2, &f_b, &f_e);

        for (int i = 0, j = 0, k = 0; i < starpu_data_get_nb_children(input_handle); i++) {
            starpu_data_handle_t sub_input = starpu_data_get_sub_data(input_handle, 1, i);
            if (not_top_level)
                starpu_data_unpartition(sub_input, STARPU_MAIN_RAM);
            starpu_data_handle_t sub_output = starpu_data_get_sub_data(output_handle, 2, j, k);

            submit_reduction_task(&sub_input, &sub_output);

            if (k + 1 == block_size) {
                k = 0;
                j++;
            } else {
                k++;
            }
        }

        not_top_level = true;
        input_handle = output_handle; // replace inputs vector by the smaller output vector
    }

    //--------------------------------------------------------------------------
    // and ends here
    //--------------------------------------------------------------------------

    starpu_task_wait_for_all();
    // starpu_data_unregister(vec_handle);
    // starpu_data_unregister(vec_output_handle);
    starpu_shutdown();

    double ts1 = get_time();
    double elapsed = ts1 - ts0;
    printf("start: %.4f\nend: %.4f\nelapsed: %.4f\n", ts0, ts1, elapsed);
}
