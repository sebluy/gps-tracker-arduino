#include <stdint.h>
#include <avr/eeprom.h>

#include "waypoint_reader.h"

/* See waypoint writer for details on waypoint path layout in memory */

void waypoint_reader_initialize(waypoint_reader_t *reader)
{
    /* count stored at address 0x0 */
    reader->count = eeprom_read_dword((uint32_t*)0x0);
    /* waypoints start at address 0x4 onward */
    reader->ptr = (float*)0x4;
}

point_t waypoint_store_get_next(waypoint_store_t *store)
{
    float latitude = eeprom_read_float(store->ptr++);
    float longitude = eeprom_read_float(store->ptr++);
    return (point_t){latitude, longitude};
}

boolean waypoint_store_end(waypoint_store_t *store)
{
    return store->ptr == (float*)(0x4 + store->count*8);
}
