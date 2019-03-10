#ifndef GBCC_TIME_H
#define GBCC_TIME_H

#include <stdint.h>
#include <time.h>

#define SECOND 1000000000u
#define MINUTE 60000000000u
#define HOUR 3600000000000u
#define DAY 86400000000000u

uint64_t gbcc_time_diff(const struct timespec *cur, const struct timespec *old);

#endif /* GBCC_TIME_H */
