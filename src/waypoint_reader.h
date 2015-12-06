#ifndef WAYPOINT_READER_H
#define WAYPOINT_READER_H

#include <stdint.h>
#include "Arduino.h"

#include "types.h"

/* A waypoint reader is used for reading waypoints from non-volatile storage.
   Use a waypoint writer to write waypoints to storage. */

struct waypoint_reader_t {
    uint8_t count;
    uint8_t valid;
    float *ptr;
};

void waypoint_reader_initialize(waypoint_reader_t *reader);

uint32_t waypoint_reader_count(waypoint_reader_t *reader);

/* Reads the next waypoint from store.
   Reader state is modified so that repeated calls return
   successive waypoints.
   This call does not check its own bounds, that responsibility
   is the users (using end). */
point_t waypoint_reader_get_next(waypoint_reader_t *reader);

/* Returns true iff there are no more waypoints to be read. */
boolean waypoint_reader_end(waypoint_reader_t *reader);

#endif
