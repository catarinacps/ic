#include "utils/utils.h"

int main(int argc, char** argv)
{
    if (argc != 3) {
        V_PRINTF("Please provide the following parameters:\n"
                 "%s <problem_size> <max_rand_value>\n",
            argv[0]);
        exit(-1);
    }

    const ullint n_elements = atoll(argv[1]);
    const uint max_value = atoi(argv[2]); // max random value for vector init

    V_PRINTF("There are %llu elements.\n", n_elements);

    ullint* vector = malloc(n_elements * sizeof(ullint));

    if (!vector) {
        V_PERROR("Bad alloc");
        exit(-2);
    }

    // initialize the vector
    for (ullint i = 0; i < n_elements; i++)
        vector[i] = generate_random_int(max_value, 0);

    int output = 0;

    double start = get_time();

    // actual accumulation
    for (ullint i = 0; i < n_elements; i++)
        output += vector[i];

    double end = get_time();

    double elapsed = start - end;
    V_PRINTF("start: %.5f\nend: %.5f\n", start, end);
    V_PRINTF("result: %llu\n", output);
    printf("%.5f", elapsed);

    return 0;
}
