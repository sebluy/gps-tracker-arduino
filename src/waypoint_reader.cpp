#include <stdint.h>
#include <avr/eeprom.h>
#include "Arduino.h"

#include "waypoint_reader.h"

/* See waypoint writer for details on waypoint path layout in memory */

void waypoint_reader_initialize(waypoint_reader_t *reader)
{
    /* count stored at address 0x0 */
    reader->valid = eeprom_read_byte((uint8_t*)0x0);
    reader->count = eeprom_read_byte((uint8_t*)0x1);
    /* waypoints start at address 0x4 onward */
    reader->ptr = (float*)0x4;
}

uint32_t waypoint_reader_count(waypoint_reader_t *reader)
{
    return reader->valid ? reader->count : 0;
}

point_t waypoint_reader_get_next(waypoint_reader_t *reader)
{
    float latitude = eeprom_read_float(reader->ptr++);
    float longitude = eeprom_read_float(reader->ptr++);
    return (point_t){latitude, longitude};
}

boolean waypoint_reader_end(waypoint_reader_t *reader)
{
    return reader->ptr == (float*)(0x4 + waypoint_reader_count(reader)*8);
}
