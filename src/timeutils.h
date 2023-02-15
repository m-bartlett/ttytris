#ifndef TIMEUTILS_H
#define TIMEUTILS_H

#include <time.h>

typedef struct timespec timespec_t;

extern inline uint32_t timer_get_elapsed_microseconds(timespec_t *A, struct timespec *B);
extern inline void timer_unset(timespec_t *timer);
extern inline bool timer_is_null(timespec_t *timer);
extern inline void timer_set_current_time(timespec_t *timer);
extern inline uint32_t timer_get_as_microseconds(timespec_t *timer);


#endif