#ifndef GPS_H
#define GPS_H

#include <stdint.h>

#include "types.h"

#define NMEA_LINE_LENGTH 80

struct gps_t {
    char nmea[NMEA_LINE_LENGTH + 1];
    int index;
};

struct gps_data_t {
    point_t location; /*!< (latitude, longitude) coordinate */
    float speed; /*!< speed in mph */
};

void gps_boot(void);
void gps_standby(void);
void gps_initialize(gps_t *gps);

uint8_t gps_available(gps_t *gps);
uint8_t gps_valid(gps_t *gps);

void gps_parse(gps_t *gps, gps_data_t *data);

#endif
