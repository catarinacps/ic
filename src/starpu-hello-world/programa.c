#include <starpu.h>

// my codelet

void cpu_func(void *buffers[], void *cl_arg)
{
    printf("Hello world\n");
}

struct starpu_codelet cl =
{
    .cpu_funcs = { cpu_func },
    .nbuffers = 0
};

int main (int argc, char **argv)
{
  int res = starpu_init(NULL);

  struct starpu_task *task = starpu_task_create();
  task->cl = &cl;
  task->synchronous = 1;
    
  starpu_task_submit(task);
  starpu_shutdown();
  return 0;
}
