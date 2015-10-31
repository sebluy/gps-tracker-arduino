#include "Arduino.h"
#include "haversine.h"

/* coordinates in radians */
double distance_between_r(double lat_a, double lng_a,
                        double lat_b, double lng_b)
{
  double a, c, del_lat, del_lng ;

  /* use haversine formula */
  del_lat = fabs(lat_a - lat_b) ;
  del_lng = fabs(lng_a - lng_b) ;

  a = 2*asin(sqrt(square(sin(del_lat/2))
        + cos(lat_a)*cos(lat_b)*square(sin(del_lng/2)))) ;

  /* alternative form */
  /*
  a = square(sin(del_lat/2)) + cos(lat_a)*cos(lat_b)*square(sin(del_lng/2)) ;
  c = 2.0*atan2(sqrt(a), sqrt(1.0 - a)) ;
  */

  return a*MEAN_EARTH_RADIUS ;
}

/* make a point struct to reduce passing */
/* a is in degrees, b is in gppga format */
double distance_between_d(double lat_a, double lng_a,
                           double lat_b, double lng_b)
{
    return distance_between_r(degrees_to_radians(lat_a),
                              degrees_to_radians(lng_a),
                              degrees_to_radians(lat_b),
                              degrees_to_radians(lng_b));
}

double degrees_to_radians(double degrees)
{
    return degrees*2*M_PI/360.0;
}
