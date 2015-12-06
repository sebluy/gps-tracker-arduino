#include <avr/eeprom.h>
#include <stdint.h>
#include <stdlib.h>

#include "waypoint_writer.h"

#define INVALID 0
#define VALID 1

#define MAX_WAYPOINTS 50

/* Waypoints are stored in EEPROM as follows:

   Count is the number of (latitude, longitude) pairs.

   For count = n where n <= MAX_WAYPOINTS

   0x00 valid (1 byte)
   0x01 n (count) (1 byte)
   0x04 1st Waypoint Latitude (4 bytes)
   0x08 1st Waypoint Longitude (4 bytes)
   0x0C 2nd Waypoint Latitude (4 bytes)
   0x10 2nd Waypoint Longitude (4 bytes)
   ...
   0x?? nth Waypoint Latitude (4 bytes)
   0x?? nth Waypoint Longitude (4 bytes)

*/

void waypoint_writer_initialize(waypoint_writer_t *writer)
{
    writer->field = COUNT;
}

/* Converts field argument to the appropriate value
   and inserts it into the next address in EEPROM. */
waypoint_writer_status_t
waypoint_writer_write(waypoint_writer_t *writer, char *field)
{
    uint8_t count;
    float latitude, longitude;
    switch (writer->field) {
    case COUNT:
        count = (uint8_t)atoi(field);
        if (count > MAX_WAYPOINTS) {
            return FAILURE;
        }
        eeprom_write_byte((uint8_t*)0x0, INVALID);
        eeprom_write_byte((uint8_t*)0x1, count);
        writer->count = count;
        writer->ptr = (float*)0x4;
        writer->field = LATITUDE;
        break;
    case LATITUDE:
        latitude = atof(field);
        eeprom_write_float(writer->ptr, latitude);
        writer->ptr++;
        writer->field = LONGITUDE;
        break;
    case LONGITUDE:
        longitude = atof(field);
        eeprom_write_float(writer->ptr, longitude);
        writer->ptr++;
        writer->field = LATITUDE;
        break;
    }

    /* return a status based on how many points have been written */
    uint8_t written = ((uint32_t)writer->ptr - 0x4)/8;
    if (written == writer->count) {
        eeprom_write_byte((uint8_t*)0x0, VALID);
        return SUCCESS;
    } else if (written > writer->count) {
        return FAILURE;
    } else {
        return IN_PROGRESS;
    }
}
