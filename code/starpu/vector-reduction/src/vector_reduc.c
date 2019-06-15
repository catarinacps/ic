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

void merge_sum(void** buffers, void* cl_arg)
{
    int somatorio = 0;
    merge_params_t* par = (merge_params_t*)cl_arg;

    printf("%d\n", par->nbuffers);

    for (int i = 0; i < par->nbuffers; i++) {
        int* valor = (int*)STARPU_VECTOR_GET_PTR(buffers[i]);
        printf("%d %d\n", i, *valor);

        somatorio += *valor;
    }

    printf("======> Sum = %d\n", somatorio);
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

int submit_reduction_task(int blksize, int blkid, starpu_data_handle_t* input_handle, starpu_data_handle_t* output_handle)
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

    // return the error code
    return starpu_task_submit(task);
}

int main(int argc, char** argv)
{
    if (argc != 4) {
        printf("Please provide the following parameters:\n"
               "%s <problem_size> <n_blocks> <degree>\n",
            argv[0]);
        exit(-1);
    }

    unsigned int nx = atoi(argv[1]);
    unsigned int n_blocks = atoi(argv[2]);
    unsigned int degree = atoi(argv[3]);

    if (degree == 1) {
        printf("Please insert a degree of reduction bigger than 1\n");
        exit(-1);
    }

    // calculating the block_size
    unsigned int block_size = nx / n_blocks;
    printf("There are %d blocks, each one with %d elements.\n",
        n_blocks, block_size);

    unsigned int reduction_merge_depth = merge_depth(n_blocks, degree);

    int ec = starpu_init(NULL);
    starpu_profiling_status_set(STARPU_PROFILING_DISABLE);

    double ts0 = get_time();

    // INPUT
    starpu_data_handle_t input_handle[n_blocks];
    int* vectors[n_blocks];

    // OUTPUT
    starpu_data_handle_t output_elements_handle[n_blocks];
    // Yes I know that's a lazy solution but hey
    int initial_reduction[n_blocks + degree];
    memset(initial_reduction, 0, (n_blocks + degree) * sizeof(int));

    for (int blkid = 0; blkid < n_blocks; blkid++) {
        vectors[blkid] = alloc_and_register_integer_vector(&input_handle[blkid], block_size, 0);

        for (int i = 0; i < block_size; i++) {
            vectors[blkid][i] = generate_random_int(MAX_RAND, MIN_RAND);
            printf("rand = %d\n", vectors[blkid][i]);
        }

        register_integer_variable(&output_elements_handle[blkid], &initial_reduction[blkid]);

        submit_reduction_task(block_size, blkid, &input_handle[blkid], &output_elements_handle[blkid]);
    }

    starpu_task_wait_for_all();

    // MERGE
    int n_inputs = n_blocks;

    int depth = merge_depth(n_blocks, degree);
    printf("depth = %d\n", depth);
    int n_merges = (int)ceil((float)n_inputs / degree);
    printf("n_merges = %d\n", n_merges);
    int* merge_inputs = initial_reduction;
    int* merge_outputs;

    starpu_data_handle_t* merge_input_handles;

    for (int i = 0; i < depth; i++) {
        starpu_data_handle_t* merge_output_handles = malloc(sizeof(starpu_data_handle_t) * n_merges);

        merge_input_handles = malloc(sizeof(starpu_data_handle_t) * n_merges);
        merge_outputs = (int*)malloc(sizeof(int) * (n_merges + degree));
        memset(merge_outputs, 0, (n_merges + degree) * sizeof(int));

        for (int k = 0, j = 0; j < n_inputs; j += degree, k++) {
            register_integer_vector(&merge_input_handles[k], &merge_inputs[j], degree);
            register_integer_variable(&merge_output_handles[k], &merge_outputs[k]);

            //create an output handle, put in an output vector
            //register new_outputs[k]
            submit_reduction_task(degree, k, &merge_input_handles[k], &merge_output_handles[k]);
        }

        //replace inputs vector by the smaller output vector
        merge_inputs = merge_outputs;
        n_inputs = n_merges;
        n_merges = (int)ceil((float)n_inputs / degree);
    }

    starpu_task_wait_for_all();
    //  starpu_data_unregister(vec_handle);
    //  starpu_data_unregister(vec_output_handle);
    starpu_shutdown();

    double ts1 = get_time();
    double elapsed = ts1 - ts0;
    printf("start: %.4f\nend: %.4f\nelapsed: %.4f\n", ts0, ts1, elapsed);
}

/* enum starpu_data_access_mode modes[n_blocks]; */
/* for (int i = 0; i < n_blocks; i++) { */
/*     modes[i] = STARPU_R; */
/* } */
/* merge_cl.dyn_modes = modes; */
/* merge_cl.nbuffers = n_blocks; */

/* // Parameters for the merge */
/* merge_params_t params; */

/* // prepare the parameters */
/* params.nbuffers = n_blocks; */

/* struct starpu_task* task = starpu_task_create(); */
/* task->synchronous = 0; */
/* task->cl = &merge_cl; */
/* task->cl_arg = &params; */
/* task->cl_arg_size = sizeof(merge_params_t); */
/* task->dyn_handles = malloc(task->cl->nbuffers * sizeof(starpu_data_handle_t)); */
/* for (int i = 0; i < task->cl->nbuffers; i++) { */
/*     task->dyn_handles[i] = output_handle[i]; */
/* } */
/* int ec2 = starpu_task_submit(task); */
