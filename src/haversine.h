/*!
 * @file
 *
 * @brief Header file for the Haversine interface
 *
 * @author Andrew Hayford
 * @author Sebastian Luy
 *
 * @date 11 November, 2015
 *
 * This file contains the prototype(s) of functions required
 * to implement the Haversine formula
 * 
 */

#ifndef HAVERSINE_H
#define HAVERSINE_H

#include "types.h"

/*!
 * @brief Calculates the distance between two points
 *
 * This function takes in two points a and b(values in degrees) 
 * and calculates the distance between them.
 *
 * @param[in]  a  A point in degrees
 * @param[in]  b  A point in degrees
 *
 * @returns    Distance in metres between the two points
 *
 */
double distance_between(point_t a, point_t b);

#endif
