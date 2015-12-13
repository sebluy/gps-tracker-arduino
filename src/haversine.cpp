/*!
 * @file
 *
 * @brief Interface for haversine formula
 *
 * @author Andrew Hayford
 * @author Sebastian Luy
 *
 * @date 12 December, 2015
 *
 * This file contains a C++ implementation of the Haversine formula
 * for use in calculating distance between two GPS coordinates
 *
 */

#include <math.h>

#include "haversine.h"
#include "types.h"

#define MEAN_EARTH_RADIUS 6371e3  /*!< Mean radius of Earth */
#define M_TO_FT 3.28084           /*!< Conversion factor for metres to feet */

/*!
 * @brief Calculates the distance between two radian coordinates
 *
 * This function implements the Haversine formula for calculating the distance
 * between two points with coordinates in radians. Returned value is in metres.
 *
 * @param[in]  a    A GPS point in units of radians
 * @param[in]  b    A GPS point in units of radians
 *
 * @returns    A double value indicating the distance between a and b in metres
 *
 */
double distance_between_radians(point_t a, point_t b)
{
  double d, del_lat, del_lng ;

  /* use haversine formula */
  /* https://en.wikipedia.org/wiki/Haversine_formula */

  del_lat = fabs(a.latitude - b.latitude) ;
  del_lng = fabs(a.longitude - b.longitude) ;

  d = 2*asin(sqrt(square(sin(del_lat/2))
        + cos(a.latitude)*cos(b.latitude)*square(sin(del_lng/2)))) ;

  return d*MEAN_EARTH_RADIUS ;
}

/*!
 * @brief Converts degrees to radians
 *
 * This function takes in a single value in degrees and
 * converts it into radians
 *
 * @param[in]  degrees  Double value indicating a measurement in degrees
 *
 * @returns    Double value indicating a measurement in radians
 *
 */
double degrees_to_radians(double degrees)
{
    return degrees*2*M_PI/360.0;
}


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
double distance_between(point_t a, point_t b)
{
    return distance_between_radians(
        (point_t){degrees_to_radians(a.latitude),
                  degrees_to_radians(a.longitude)},
        (point_t){degrees_to_radians(b.latitude),
                  degrees_to_radians(b.longitude)});
}
