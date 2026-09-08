#ifndef ALURA_SYS_TIMES_H
#define ALURA_SYS_TIMES_H

#include <stdint.h>

typedef int32_t clock_t;

struct tms {
    clock_t tms_utime;
    clock_t tms_stime;
    clock_t tms_cutime;
    clock_t tms_cstime;
};

#endif // ALURA_SYS_TIMES_H
