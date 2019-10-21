#include "utils.h"

double get_time(void)
{
    struct timeval tr;
    gettimeofday(&tr, NULL);
    return (double)tr.tv_sec + (double)tr.tv_usec / 1000000;
}

llint generate_random_int(const int max, const int min)
{
    return rand() % (max + 1 - min) + min;
}
