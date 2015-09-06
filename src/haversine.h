#ifndef HAVERSINE_H
#define HAVERSINE_H

#ifdef _cpluscplus
extern "C" {
#endif

#define MEAN_EARTH_RADIUS 6371e3
#define M_TO_FT 3.28084

double calc_dist_coord(double lat_a, double lng_a, 
                       double lat_b, double lng_b) ;

double GPPGA_to_radians(double degrees) ;

#ifdef _cplusplus
}
#endif

#endif
