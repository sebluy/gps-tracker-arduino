#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

struct point_t {
    double latitude;
    double longitude;
};

struct tracking_record_t {
    int num_points;
    double aggregate_speed;
    point_t previous_point;
    uint32_t start_time; /* unix epoch time */
};

struct tracking_data_t {
    double instant_speed;
    double time_elapsed;
    double average_speed;
    double total_distance;
    double waypoint_distance;
};

#endif
