#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef VERBOSE
#define V_PRINTF(f_, ...) printf((f_), ##__VA_ARGS__)
#define V_PERROR(f_) perror((f_))
#else
#define V_PRINTF(f_, ...) ((void)0)
#define V_PERROR(f_) ((void)0)
#endif

typedef unsigned int uint;
typedef unsigned long long ullint;
typedef long long llint;

llint generate_random_int(const int max, const int min);

double get_time(void);
