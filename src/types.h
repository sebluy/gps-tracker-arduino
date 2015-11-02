#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

/* break up declaration and typedef because arduino is dumb */
struct point_t {
    double latitude;
    double longitude;
};
typedef struct point_t point_t;

struct tracking_record_t {
    int num_points;
    double aggregate_speed;
    point_t previous_point;
    uint32_t start_time; /* unix epoch time */
};
typedef struct tracking_record_t tracking_record_t;

struct tracking_data_t {
    double instant_speed;
    double time_elapsed;
    double average_speed;
    double total_distance;
    double waypoint_distance;
};
typedef struct tracking_data_t tracking_data_t;

#endif
