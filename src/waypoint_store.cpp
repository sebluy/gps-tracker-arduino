#include <stdint.h>
#include <avr/eeprom.h>

#include "waypoint_store.h"

void waypoint_store_initialize(waypoint_store_t *store)
{
    /* count stored at address 0x0 */
    store->count = eeprom_read_dword((uint32_t*)0x0);
    /* waypoints start at address 0x4 onward */
    store->ptr = (float*)0x4;
}

/* gets the next waypoint from store. store state is modified
   so that repeated calls return successive waypoints */
point_t waypoint_store_get_next(waypoint_store_t *store)
{
    float latitude = eeprom_read_float(store->ptr++);
    float longitude = eeprom_read_float(store->ptr++);
    return (point_t){latitude, longitude};
}

/* returns 0 if their are more waypoints in store and
   1 otherwise */
uint8_t waypoint_store_end(waypoint_store_t *store)
{
    return store->ptr == (float*)(0x4 + store->count*8);
}
