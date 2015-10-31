#ifndef HAVERSINE_H
#define HAVERSINE_H

#ifdef _cpluscplus
extern "C" {
#endif

#define MEAN_EARTH_RADIUS 6371e3
#define M_TO_FT 3.28084

/* coordinates in radians */
double distance_between_r(double lat_a, double lng_a,
                          double lat_b, double lng_b);

/* make a point struct to reduce passing */
/* coordinates are in degrees */
double distance_between_d(double lat_a, double lng_a,
                           double lat_b, double lng_b);

double GPPGA_to_radians(double gppga);

double degrees_to_radians(double degrees);

#ifdef _cplusplus
}
#endif

#endif
