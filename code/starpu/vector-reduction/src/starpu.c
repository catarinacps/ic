#include <math.h>
#include <starpu.h>
#include <stdbool.h>

#include "helpers/starpu_helpers.h"
#include "reduc/vector_reduc.h"
#include "utils/utils.h"

int main(int argc, char** argv)
{
    if (argc != 5) {
        V_PRINTF("Please provide the following parameters:\n"
                 "%s <problem_size> <number_blocks> <decay_factor> <max_rand_value>\n",
            argv[0]);
        exit(-1);
    }

    //--------------------------------------------------------------------------
    // cli arguments
    //--------------------------------------------------------------------------

    const ullint original_n_elements = atoll(argv[1]);
    ullint n_blocks = atoi(argv[2]);
    const ullint decay_factor = atoll(argv[3]);
    const uint max_value = atoi(argv[4]); // max random value for vector init

    ullint block_size = (ullint)ceil((double)original_n_elements / n_blocks);

    V_PRINTF("number of blocks = %d\n", n_blocks);

    ullint n_elements = n_blocks * block_size;

    if (n_blocks < 1 || n_blocks > n_elements) {
        V_PRINTF("Please provide a number of blocks bigger than 0 and smaller than the number of elements\n");
        exit(-1);
    }

    if (decay_factor > n_blocks || decay_factor <= 1) {
        V_PRINTF("the decay factor must be smaller or equal to the number of blocks and bigger than 1\n");
        exit(-1);
    }

    V_PRINTF("There are %llu blocks, each one with %llu elements.\n", n_blocks, block_size);

    int ec = starpu_init(NULL);
    starpu_profiling_status_set(STARPU_PROFILING_DISABLE);

    //--------------------------------------------------------------------------
    // the algorithm actually starts here
    //--------------------------------------------------------------------------

    // INPUT
    starpu_data_handle_t input_handle;
    llint* input_vector = alloc_and_register_integer_vector(&input_handle, n_elements);

    if (!input_vector) {
        V_PERROR("Bad malloc");
        exit(-2);
    }

    // initialize the vector
    for (ullint i = 0; i < original_n_elements; i++)
        input_vector[i] = generate_random_int(max_value, 0);

    // start timestamp
    double ts0 = get_time();

    // OUTPUT
    starpu_data_handle_t output_handle;

    partition_vector_handle(&input_handle, n_blocks);

    uint depth = 0;
    bool not_top_level = false;
    while (n_blocks >= 1 && n_elements > 1) {
        V_PRINTF("depth = %d\n"
                 "block size = %d\n"
                 "number of blocks = %d\n"
                 "number of elements = %d\n",
            depth++, block_size, n_blocks, n_elements);

        n_elements = n_blocks;
        n_blocks = (ullint)ceil((double)n_blocks / decay_factor);
        block_size = (ullint)ceil((double)n_elements / n_blocks);
        n_elements = n_blocks * block_size;

        // here we don't need the actual vector as we won't be initializing it
        // to any arbitrary value
        llint* alloc_return = alloc_and_register_integer_vector(&output_handle, n_elements);

        if (!alloc_return) {
            V_PERROR("Bad malloc");
            exit(-2);
        }

        starpu_data_filter_t f_b = { .filter_func = starpu_vector_filter_block, .nchildren = n_blocks };
        starpu_data_filter_t f_e = { .filter_func = starpu_vector_filter_block, .nchildren = block_size };

        starpu_data_map_filters(output_handle, 2, &f_b, &f_e);

        for (ullint i = 0, j = 0, k = 0; (int) i < starpu_data_get_nb_children(input_handle); i++) {
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
        free(input_vector);
        input_vector = alloc_return;
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
    V_PRINTF("start: %.5f\nend: %.5f\n", ts0, ts1);
    printf("%.5f", elapsed);

    free(input_vector);

    return 0;
}
