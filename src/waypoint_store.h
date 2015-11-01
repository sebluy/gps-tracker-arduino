#ifndef WAYPOINT_STORE_H
#define WAYPOINT_STORE_H

#include <stdint.h>
#include "types.h"

typedef struct {
    uint32_t count;
    float *ptr;
} waypoint_store_t;

/* prepare waypoint store for usage */
void waypoint_store_initialize(waypoint_store_t *store);

/* gets the next waypoint from store. store state is modified
   so that repeated calls return successive waypoints */
point_t waypoint_store_get_next(waypoint_store_t *store);

/* returns 0 if their are more waypoints in store and
   1 otherwise */
uint8_t waypoint_store_end(waypoint_store_t *store);

#endif
