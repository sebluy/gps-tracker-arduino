#ifndef TIME_H
#define TIME_H

#include <stdint.h>

uint32_t get_unix_time(uint32_t hh, uint32_t mm, uint32_t ss,
                       uint32_t dd, uint32_t mo, uint32_t yy);

#endif
