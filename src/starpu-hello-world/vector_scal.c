#include <stdio.h>
#include <stdlib.h>
#include <starpu.h>
#include <sys/time.h>

struct params
{
  unsigned int begin;
  unsigned int end;
  int factor;
};

double get_time (void)
{
  struct timeval tr;
  gettimeofday(&tr, NULL);
  return (double)tr.tv_sec+(double)tr.tv_usec/1000000;
}

void cpu_func(void **buffers, void* cl_arg)
{
  //get data handles
  int *vec_input = (int*) STARPU_VECTOR_GET_PTR(buffers[0]);
  int *vec_output = (int*) STARPU_VECTOR_GET_PTR(buffers[1]);
  unsigned nx_input = STARPU_VECTOR_GET_NX(buffers[0]);
  unsigned nx_output = STARPU_VECTOR_GET_NX(buffers[1]);

  //get parameters
  struct params *par = (struct params*) cl_arg;

  double t0 = get_time();
  printf("%f Task started %p - %p, begin=%d, end=%d\n", t0,
	 vec_input, vec_output, par->begin, par->end);

  //do the job
  for(int i = 0; i < (par->end - par->begin); i++) {
    vec_output[i] = vec_input[i] + par->factor;
//    for (int l = 0; l < 100; l++){ double x = x * 2; }
  }

  double t1 = get_time();
  printf("%f Task finished to work with begin=%d (%f)\n", t1, par->begin, t1-t0);
}

struct starpu_codelet cl =
{
  .where = STARPU_CPU,
  .cpu_funcs = {cpu_func},
  .nbuffers = 2,
  .modes = {STARPU_R, STARPU_W }
};


starpu_data_handle_t alloc_one_vector(int block_size, int init_value)
{
  starpu_data_handle_t handle;
  int *vec = (int*) malloc(block_size * sizeof(int));
  for(int j = 0; j < block_size; j++){
    vec[j] = init_value;
  }
#ifdef DEBUG
  printf("%s %p\n", __FUNCTION__, vec);
#endif
  //register with starpu
  starpu_vector_data_register(&handle,
			      STARPU_MAIN_RAM,
			      (uintptr_t)vec,
			      block_size,          //number of elements
			      sizeof(vec[0]));//size of the type of elements
  return handle;
}

starpu_data_handle_t create_and_submit_task (int blkid,
					     unsigned int block_size,
					     int factor,
					     starpu_data_handle_t vec_handle)
{
  struct params* params;
  params = (struct params*) malloc(sizeof(struct params));

  //prepare the parameters
  params->begin = block_size * blkid;
  params->end = block_size * blkid + block_size;
  params->factor = factor;

  printf("Creating task %d with block_size = %d (%d - %d)\n",
	 blkid, block_size, params->begin, params->end);

  //create vec_output_handle
  starpu_data_handle_t vec_output_handle = alloc_one_vector(block_size, 0);

#ifdef DEBUG
  printf("%s %p\n", __FUNCTION__,
	 starpu_data_get_local_ptr(vec_output_handle));
#endif

  struct starpu_task *task = starpu_task_create();
  task->synchronous = 0;
  task->cl =&cl;
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

starpu_data_handle_t *alloc_vectors (int n_blocks,
				     int block_size,
				     int init_value)
{
  starpu_data_handle_t *handle;
  handle = (starpu_data_handle_t*) malloc(n_blocks * sizeof(starpu_data_handle_t));

  for(int blkid = 0; blkid < n_blocks; blkid++) {
    int *vec = (int*) malloc(block_size * sizeof(int));
    for(int j = 0; j < block_size; j++){
      vec[j] = init_value;
    }

    //register with starpu
    starpu_vector_data_register(&handle[blkid],
				STARPU_MAIN_RAM,
				(uintptr_t)vec,
				block_size,          //number of elements
				sizeof(vec[0]));//size of the type of elements
  }
  return handle;
}



int main(int argc, char** argv)
{
  if(argc != 4) {
    printf("Please, provide the following parameters:\n"
	   "%s <problem_size> <n_blocks> <factor>\n", argv[0]);
    exit(0);
  }

  unsigned int nx;         //the problem size
  unsigned int n_blocks;   //the number of blocks (defines the granularity)
  int factor;              //the multiplicative factor
  unsigned int block_size; //the block size (granularity)

  //reading parameters from the command-line
  nx = atoi(argv[1]);
  n_blocks = atoi(argv[2]);
  factor = atoi(argv[3]);

  //calculating the block_size
  block_size = nx / n_blocks;

  printf("There are %d blocks, each one with %d elements.\n",
	 n_blocks, block_size);

  int ec;
  ec = starpu_init(NULL);

  /* //allocate and initialize the input vector */
  starpu_data_handle_t *vec_handle;
  vec_handle = alloc_vectors (n_blocks, block_size, 1);

//  starpu_data_handle_t *vec_output_handle;
//  vec_output_handle = alloc_vectors (n_blocks, block_size, 0);

  double ts0 = get_time();

  //submit the tasks that do the job
//  starpu_data_handle_t input = alloc_one_vector(block_size, 1);

  for (int ts = 0; ts < 10; ts++){
    starpu_data_handle_t output;
    for(int blkid = 0; blkid < n_blocks; blkid++) {
      starpu_data_handle_t output = create_and_submit_task(blkid,
							   block_size,
							   factor,
							   vec_handle[blkid]);
      vec_handle[blkid] = output;
    }
  }

  starpu_task_wait_for_all();
//  starpu_data_unregister(vec_handle);
//  starpu_data_unregister(vec_output_handle);
  starpu_shutdown();

  double ts1 = get_time();
  double elapsed = ts1 - ts0;
  printf("%f %f %f\n", ts0, ts1, elapsed);

#ifdef VALIDATION_PRINT
  /* for(int blkid = 0; blkid < n_blocks; blkid++) { */
  /*   for (int i; i < block_size; i++){ */
  /*     if (vec_output[blkid][i] == factor + 1){ */
  /* 	printf("Error, value (%d) of vector position %d is incorrect. " */
  /* 	     "It should be %d.\n", vec_output[i], i, factor); */
  /*     } */
  /*   } */
  /* } */
#endif
}
