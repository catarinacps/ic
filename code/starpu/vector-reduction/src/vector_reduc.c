#include "vector_reduc.h"

double get_time(void)
{
    struct timeval tr;
    gettimeofday(&tr, NULL);
    return (double)tr.tv_sec + (double)tr.tv_usec / 1000000;
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
    int* vec_output = (int*)STARPU_VECTOR_GET_PTR(buffers[1]);
    unsigned nx_input = STARPU_VECTOR_GET_NX(buffers[0]);
    unsigned nx_output = STARPU_VECTOR_GET_NX(buffers[1]);

    reduc_params_t* par = (reduc_params_t*)cl_arg;

    double t0 = get_time();
    /* printf("%f Task started %p - %p, begin=%d, end=%d\n", */
    /* 	   t0, vec_input, vec_output, par->begin, par->end); */

    // do the job
    for (int i = 0; i < (par->end - par->begin); i++) {
        vec_output[0] += vec_input[i];
    }

    double t1 = get_time();
    /* printf("%f Task finished to work with begin=%d (%f)\n", */
    /* 	   t1, par->begin, t1 - t0); */
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        printf("Please provide the following parameters:\n"
               "%s <problem_size> <n_blocks>\n",
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

    // INPUT
    starpu_data_handle_t input_handle[n_blocks];

    for (int i = 0; i < n_blocks; i++) {
        int* vec = (int*)malloc(block_size * sizeof(int));

        for (int j = 0; j < block_size; j++) {
            vec[j] = 2;
        }

        // register with starpu
        starpu_vector_data_register(
            &input_handle[i],
            STARPU_MAIN_RAM,
            (uintptr_t)vec,
            block_size, // number of elements
            sizeof(int)); // size of the type of elements
    }

    double ts0 = get_time();

    // OUTPUT
    starpu_data_handle_t output_handle[n_blocks];

    for (int i = 0; i < n_blocks; i++) {
        int* vec = (int*)malloc(sizeof(int));

        *vec = 0;

        // register with starpu
        starpu_vector_data_register(
            &output_handle[i],
            STARPU_MAIN_RAM,
            (uintptr_t)vec,
            block_size, // number of elements
            sizeof(int)); // size of the type of elements
    }

    for (int blkid = 0; blkid < n_blocks; blkid++) {

        reduc_params_t* params;
        params = (reduc_params_t*)malloc(sizeof(reduc_params_t));

        // prepare the parameters
        params->begin = block_size * blkid;
        params->end = block_size * blkid + block_size;

        struct starpu_task* task = starpu_task_create();
        task->synchronous = 0;
        task->cl = &reduc_cl;
        task->cl_arg = params;
        task->cl_arg_size = sizeof(reduc_params_t);
        task->handles[0] = input_handle[blkid];
        task->handles[1] = output_handle[blkid];

        int ec = starpu_task_submit(task);
    }

    // input vector is equal to the output_handle.
    starpu_data_handle_t* inputs = output_handle;
    int n_inputs = n_blocks;
    while () {
        starpu_data_handle_t* new_outputs = malloc(sizeof(starpu_data_handle_t*) * n_inputs / degree);
        for (int k, i = 0; i < n_inputs; i += degree, k++) {
            //prepare the input handles according to the i index
            starpu_data_handle_t* task_inputs = malloc(sizeof(starpu_data_handle_t*) * degree);
            for (j = 0; j < degree; j++) {
                task_inputs[j] = inputs[i + j];
            }
            //create an output handle, put in an output vector
            //register new_outputs[k]
            //submit_merge_task (inputs, output)
            submit_merge_task(task_inputs, degree, new_outputs[k]);
        }
        //replace inputs vector by the smaller output vector
        inputs = new_outputs;
        n_inputs = n_blocks / degree;
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
