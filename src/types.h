/*!
 * @file
 *
 * @brief Header file for tracking mode specific types
 *
 * @author Andrew Hayford
 * @author Sebastian Luy
 *
 * @date 11 November, 2015
 *
 * This file contains structs to abstract away
 * complexity in the gps coordinates and tracking
 *
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

/*!
 * @brief struct to hold gps point consisting of a latitude and longitude
 *
 * This struct holds a latitude and longitude pair which make up a single gps
 * coordinate
 *
 */
struct point_t {
    float latitude;    /*!< Latitude of coordinate, in degrees */
    float longitude;   /*!< Longitude of coordinate, in degrees */
};

/*!
 * @brief struct to handle book-keeping while in tracking mode
 *
 * This struct holds various values to help handle book-keeping
 * during tracking mode. This includes number of points since entering
 * tracking mode, the sum (aggregate) of the speed, the current waypoint
 * in the ordered list, and the current and previous points received
 * in tracking mode.
 *
 */
struct tracking_record_t {
    int num_points;                  /*!< Number of points accumulated in tracking */
    float aggregate_speed;           /*!< Sum of speeds since entering tracking mode */
    point_t current_waypoint;        /*!< The current waypoint in the ordered list */
    point_t current_tracking_point;  /*!< The most recently received point in tracking */
    point_t previous_tracking_point; /*!< The previous point received in tracking mode */
};

/*!
 * @brief struct to store data for tracking mode
 *
 * This struct stores the tracking mode data which is displayed on the LCD.
 * This includes the speeds and distances associated with tracking, along with the
 * time and a flag indicating if the waypoint path is complete.
 *
 */
struct tracking_data_t {
    float instant_speed;       /*!< Instantaneous speed in mph */
    int time_elapsed;          /*!< Time elapsed since entering tracking mode, in seconds */
    float average_speed;       /*!< Average speed since entering tracking, in mph */
    float total_distance;      /*!< Total distance traveled since entering tracking mode, in meters */
    float waypoint_distance;   /*!< Distance to closest, non-passed waypoint in meters */
    uint8_t waypoint_done;     /*!< Flag to indicate waypoint path is complete */
};

#endif
