#ifndef GPS_H
#define GPS_H

#include <stdint.h>

#include "types.h"

#define NMEA_LINE_LENGTH 80

/* buffers data received from gps module until it is complete
   and ready to be used */
struct gps_t {
    char nmea[NMEA_LINE_LENGTH + 1];
    int index;
};

/* contains data parsed from gps module */
struct gps_data_t {
    point_t location; /* (latitude, longitude) coordinate */
    float speed; /* speed in mph */
};

/* Initializes gps and then puts in standby mode */
void gps_boot(void);

/* Called while gps is active to put device in standby. If the device is already
   in standby, this might wake it up so be careful. */
void gps_standby(void);

/* initializes a gps object (used for nmea input bookkeeping),
   wakes up the gps module and then requests updates in RMC format */
void gps_initialize(gps_t *gps);

/* returns true if valid gps nmea string is available
 * only returns true once per valid nmea string */
boolean gps_available(gps_t *gps);


/* returns true iff the gps is valid (and is ready to be parsed) */
boolean gps_valid(gps_t *gps);

/* parses last nmea string recieved into gps data.
   Should only be used after gps_valid returns true. */
void gps_parse(gps_t *gps, gps_data_t *data);

#endif
