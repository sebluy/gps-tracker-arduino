#include <avr/eeprom.h>
#include <stdint.h>
#include <stdlib.h>

#include "waypoint_writer.h"

/* Waypoints are stored in EEPROM as follows:

   Count is the number of (latitude, longitude) pairs.

   For count = n

   0x00 n (count) (4 bytes)
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
    writer->ptr = (float*)0x0;
}

/* converts field argument to the appropriate value
   and inserts it into the next address in EEPROM */
void waypoint_writer_write(waypoint_writer_t *writer, char *field)
{
    uint32_t count;
    float latitude, longitude;
    switch (writer->field) {
    case COUNT:
        count = (uint32_t)atoi(field);
        eeprom_write_dword((uint32_t*)writer->ptr, count);
        writer->ptr++;
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
}
