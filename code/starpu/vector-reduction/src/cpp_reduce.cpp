#include <vector>
#include <execution>
#include <numeric>

extern "C" {
#include "utils/utils.h"
}

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

    std::vector<ullint> vec(n_elements);

    for (auto& element : vec)
        element = generate_random_int(max_value, 0);

    double start = get_time();

    auto result = std::reduce(std::execution::par, vec.begin(), vec.end());

    double end = get_time();

    double elapsed = start - end;
    V_PRINTF("start: %.5f\nend: %.5f\n", start, end);
    V_PRINTF("result: %llu\n", result);
    printf("%.5f", elapsed);

    return 0;
}
