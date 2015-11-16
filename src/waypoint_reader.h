#ifndef WAYPOINT_READER_H
#define WAYPOINT_READER_H

#include <stdint.h>
#include "Arduino.h"

#include "types.h"

/* A waypoint reader is used for reading waypoints from non-volatile storage.
   Use a waypoint writer to write waypoints to storage. */

struct waypoint_reader_t {
    uint32_t count;
    float *ptr;
};

void waypoint_reader_initialize(waypoint_reader_t *reader);

/* Reads the next waypoint from store. Reader state is modified
   so that repeated calls return successive waypoints. */
point_t waypoint_reader_get_next(waypoint_reader_t *reader);

/* Returns true iff there are no more waypoints to be read. */
boolean waypoint_reader_end(waypoint_reader_t *reader);

#endif
