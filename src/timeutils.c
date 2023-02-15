#include <stdbool.h>
#include <stdint.h>
#include "timeutils.h"


inline uint32_t timer_get_elapsed_microseconds(timespec_t *A, struct timespec *B)
{
    return (B->tv_sec - A->tv_sec) * 1e6 + (B->tv_nsec - A->tv_nsec) / 1000;
}


inline void timer_unset(timespec_t *timer)
{
    timer->tv_sec = 0;
    timer->tv_nsec = 0;
}


inline bool timer_is_null(timespec_t *timer)
{
    return timer->tv_sec == 0 && timer->tv_nsec == 0;
}


inline void timer_set_current_time(timespec_t *timer)
{
    clock_gettime(CLOCK_MONOTONIC, timer);
}


inline uint32_t timer_get_as_microseconds(timespec_t *timer)
{
    return timer->tv_sec * 1e6 + timer->tv_nsec / 1000;
}