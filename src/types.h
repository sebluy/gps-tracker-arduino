#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

struct point_t {
    float latitude;
    float longitude;
};

struct tracking_record_t {
    int num_points;
    float aggregate_speed;
    point_t current_waypoint;
    point_t current_tracking_point;
    point_t previous_tracking_point;
};

struct tracking_data_t {
    float instant_speed; /* knots */
    int time_elapsed; /* secs */
    float average_speed; /* knots */
    float total_distance; /* meters */
    float waypoint_distance; /* meters */
    uint8_t waypoint_done;
};

#endif
