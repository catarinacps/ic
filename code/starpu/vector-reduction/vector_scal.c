#include <starpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

struct params2 {
    unsigned int nbuffers;
};
struct params {
    unsigned int begin;
    unsigned int end;
};

double get_time(void)
{
    struct timeval tr;
    gettimeofday(&tr, NULL);
    return (double)tr.tv_sec + (double)tr.tv_usec / 1000000;
}

void cpu_func2(void** buffers, void* cl_arg)
{
    int somatorio = 0;
    struct params2* par = (struct params2*)cl_arg;

    printf("%d\n", par->nbuffers);

    for (int i = 0; i < par->nbuffers; i++) {
        int* valor = (int*)STARPU_VECTOR_GET_PTR(buffers[i]);
        printf("%d %p\n", i, valor);
        printf("%d %d\n", i, *valor);
        somatorio += *valor;
    }
    printf("======> Somatorio = %d\n", somatorio);
}

void cpu_func(void** buffers, void* cl_arg)
{
    int* vec_input = (int*)STARPU_VECTOR_GET_PTR(buffers[0]);
    int* vec_output = (int*)STARPU_VECTOR_GET_PTR(buffers[1]);
    unsigned nx_input = STARPU_VECTOR_GET_NX(buffers[0]);
    unsigned nx_output = STARPU_VECTOR_GET_NX(buffers[1]);

    struct params* par = (struct params*)cl_arg;

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

struct starpu_codelet cl = { .where = STARPU_CPU,
    .cpu_funcs = { cpu_func },
    .nbuffers = 2,
    .modes = { STARPU_R, STARPU_W } };

starpu_data_handle_t alloc_one_vector(int block_size, int init_value, int* vec)
{
    starpu_data_handle_t handle;

    for (int j = 0; j < block_size; j++) {
        vec[j] = init_value;
    }

#ifdef DEBUG
    printf("%s %p\n", __FUNCTION__, vec);
#endif

    // register with starpu
    starpu_vector_data_register(&handle, STARPU_MAIN_RAM, (uintptr_t)vec,
        block_size, // number of elements
        sizeof(int)); // size of the type of elements
    return handle;
}

starpu_data_handle_t create_and_submit_task(
    int blkid,
    unsigned int block_size,
    starpu_data_handle_t vec_handle,
    starpu_data_handle_t vec_output_handle)
{
    struct params* params;
    params = (struct params*)malloc(sizeof(struct params));

    // prepare the parameters
    params->begin = block_size * blkid;
    params->end = block_size * blkid + block_size;

    /* printf("Creating task %d with block_size = %d (%d - %d)\n", blkid, block_size, */
    /*     params->begin, params->end); */

    // create vec_output_handle

#ifdef DEBUG
    printf("%s %p\n", __FUNCTION__, starpu_data_get_local_ptr(vec_output_handle));
#endif

    struct starpu_task* task = starpu_task_create();
    task->synchronous = 0;
    task->cl = &cl;
    task->cl_arg = params;
    task->cl_arg_size = sizeof(struct params);
    task->handles[0] = vec_handle;
    task->handles[1] = vec_output_handle;

#ifdef PRINT
    printf("Submiting task (%u - %d)\n", params[blkid].begin, params[blkid].end);
#endif

    int ec = starpu_task_submit(task);

    return vec_output_handle;
}

starpu_data_handle_t* alloc_vectors(int n_blocks, int block_size,
    int init_value)
{
    starpu_data_handle_t* handle;
    handle = (starpu_data_handle_t*)malloc(n_blocks * sizeof(starpu_data_handle_t));

    for (int blkid = 0; blkid < n_blocks; blkid++) {
        int* vec = (int*)malloc(block_size * sizeof(int));
        for (int j = 0; j < block_size; j++) {
            vec[j] = init_value;
        }

        // register with starpu
        starpu_vector_data_register(
            &handle[blkid],
            STARPU_MAIN_RAM,
            (uintptr_t)vec,
            block_size, // number of elements
            sizeof(int)); // size of the type of elements

        //	int *p = (int*)starpu_data_get_user_data(handle[blkid]);
        //	printf("%s %p\n", __FUNCTION__, p);
        //	printf("%s %d\n", __FUNCTION__, *p);
    }

    return handle;
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        printf("Please, provide the following parameters:\n"
               "%s <problem_size> <n_blocks>\n", argv[0]);
        exit(0);
    }

    unsigned int nx;
    unsigned int n_blocks;
    unsigned int block_size;

    nx = atoi(argv[1]);
    n_blocks = atoi(argv[2]);

    // calculating the block_size
    block_size = nx / n_blocks;

    printf("There are %d blocks, each one with %d elements.\n",
        n_blocks, block_size);

    int ec = starpu_init(NULL);
    starpu_profiling_status_set(STARPU_PROFILING_DISABLE);

    // INPUT
    starpu_data_handle_t* vec_handle;
    vec_handle = alloc_vectors(n_blocks, block_size, 2);

    double ts0 = get_time();

    // OUTPUT
    starpu_data_handle_t* vec_output_handle;
    vec_output_handle = alloc_vectors(n_blocks, 1, 0);

    for (int blkid = 0; blkid < n_blocks; blkid++) {
        create_and_submit_task(blkid,
            block_size,
            vec_handle[blkid],
            vec_output_handle[blkid]);
    }

    /* Adjust codelet */
    struct starpu_codelet cl1 = {
        .where = STARPU_CPU,
        .cpu_funcs = { cpu_func2 },
        .nbuffers = n_blocks
    };

    enum starpu_data_access_mode* modes = malloc(n_blocks * sizeof(enum starpu_data_access_mode));
    for (int i = 0; i < n_blocks; i++) {
        modes[i] = STARPU_R;
    }
    cl1.dyn_modes = modes;

    /* Adjust parameters */
    struct params2* params = (struct params2*)malloc(sizeof(struct params2));

    // prepare the parameters
    params->nbuffers = n_blocks;

    struct starpu_task* task = starpu_task_create();
    task->synchronous = 0;
    task->cl = &cl1;
    task->cl_arg = params;
    task->cl_arg_size = sizeof(struct params2);
    task->dyn_handles = malloc(task->cl->nbuffers * sizeof(starpu_data_handle_t));
    for (int i = 0; i < task->cl->nbuffers; i++) {
        task->dyn_handles[i] = vec_output_handle[i];
    }
    int ec2 = starpu_task_submit(task);

    starpu_task_wait_for_all();
    starpu_shutdown();

    //    for (int blkid = 0; blkid < n_blocks; blkid++) {
    //      starpu_data_unregister(vec_output_handle[blkid]);
    //      int *p = (int*)starpu_data_get_user_data(vec_output_handle[blkid]);
    //      printf("%d %p\n", blkid, p);
    //    }

    exit(1);

    int fim;
    starpu_data_handle_t fim_output_handle = alloc_one_vector(1, 0, &fim);

    //  create_and_submit_task(0, n_blocks, 1, vec_output_handle, fim_output_handle);

    starpu_task_wait_for_all();
    //  starpu_data_unregister(vec_handle);
    //  starpu_data_unregister(vec_output_handle);
    starpu_shutdown();

    double ts1 = get_time();
    double elapsed = ts1 - ts0;
    printf("start: %.4f\nend: %.4f\nelapsed: %.4f\n", ts0, ts1, elapsed);

    printf("fim: %d\n", fim);
}
