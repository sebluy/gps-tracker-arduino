#include <math.h>

#include "haversine.h"
#include "types.h"

#define MEAN_EARTH_RADIUS 6371e3
#define M_TO_FT 3.28084

/* coordinates in radians */
double distance_between_radians(point_t a, point_t b)
{
  double d, del_lat, del_lng ;

  /* use haversine formula */
  del_lat = fabs(a.latitude - b.latitude) ;
  del_lng = fabs(a.longitude - b.longitude) ;

  d = 2*asin(sqrt(square(sin(del_lat/2))
        + cos(a.longitude)*cos(b.longitude)*square(sin(del_lng/2)))) ;

  /* alternative form */
  /*
  d = square(sin(del_lat/2)) + cos(a.latitude)
         *cos(b.latitude)*square(sin(del_lng/2)) ;
  c = 2.0*atan2(sqrt(d), sqrt(1.0 - d)) ;
  */

  return d*MEAN_EARTH_RADIUS ;
}


double degrees_to_radians(double degrees)
{
    return degrees*2*M_PI/360.0;
}

/* a is in degrees, b is in gppga format */
double distance_between(point_t a, point_t b)
{
    return distance_between_radians(
        (point_t){degrees_to_radians(a.latitude),
                  degrees_to_radians(a.longitude)},
        (point_t){degrees_to_radians(b.latitude),
                  degrees_to_radians(b.longitude)});
}
