extern "C" {
#include <avr/eeprom.h>
#include <stdint.h>
#include <stdlib.h>
}

#include "waypoint_writer.h"

void waypoint_writer_initialize(waypoint_writer_t *writer)
{
    writer->field = COUNT;
    writer->ptr = (float*)0x0;
}

/* Writes a waypoint field to storage. After initialization should
   be called successively with count, lat1, lng1, lat2, lng2, ...
   for "count" points */
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
