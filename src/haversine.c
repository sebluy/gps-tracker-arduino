#include "Arduino.h"
#include "haversine.h"

double calc_dist_coord(double lat_a, double lng_a,
                       double lat_b, double lng_b)
{
  double a, c, del_lat, del_lng ;
  double lat_a_radians = GPPGA_to_radians(lat_a) ;
  double lng_a_radians = GPPGA_to_radians(lng_a) ;
  double lat_b_radians = GPPGA_to_radians(lat_b) ;
  double lng_b_radians = GPPGA_to_radians(lng_b) ;

  /* use haversine formula */
  del_lat = fabs(lat_a_radians - lat_b_radians) ;
  del_lng = fabs(lng_a_radians - lng_b_radians) ;

  a = 2*asin(sqrt(square(sin(del_lat/2))
        + cos(lat_a)*cos(lat_b)*square(sin(del_lng/2)))) ;

  /* alternative form */
  /*
  a = square(sin(del_lat/2)) + cos(lat_a)*cos(lat_b)*square(sin(del_lng/2)) ;
  c = 2.0*atan2(sqrt(a), sqrt(1.0 - a)) ;
  */

  return a*MEAN_EARTH_RADIUS ;
}

double GPPGA_to_radians(double degrees)
{
  double integral, fractional ;
  fractional = modf(degrees/100.0, &integral) ;
  return (integral + fractional*100.0/60.0)*2*M_PI/360.0 ;
}
