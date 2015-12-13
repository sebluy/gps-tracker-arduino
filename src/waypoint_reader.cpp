/*!
 * @file
 *
 * @brief Interface for reading waypoints from EEPROM
 *
 * @author Andrew Hayford
 * @author Sebastian Luy
 *
 * @date 12 December, 2015
 *
 * This file contains the function definitions used to read 
 * waypoints from EEPROM.
 * 
 * See waypoint writer for details on waypoint path layout in memory
 */

#include <stdint.h>
#include <avr/eeprom.h>
#include "Arduino.h"

#include "waypoint_reader.h"

/*!
 * @brief Initializes reading of waypoints from EEPROM
 *
 * This reads the valid flag in EEPROM, reads in the 
 * number of expected waypoints, then places the 
 * pointer at the base address of the waypoints list
 *
 * @param[in,out] reader  Pointer to reader struct with valid flag, count, EEPROM pointer
 *
 * @returns    Nothing.
 *
 */
void waypoint_reader_initialize(waypoint_reader_t *reader)
{
    /* count stored at address 0x0 */
    reader->valid = eeprom_read_byte((uint8_t*)0x0);
    reader->count = eeprom_read_byte((uint8_t*)0x1);
    /* waypoints start at address 0x4 onward */
    reader->ptr = (float*)0x4;
}


/*!
 * @brief Checks valid flag and assigns number of values to read
 *
 * This checks the valid flag in the reader struct. If valid,
 * the number of points to read from EEPROM is set to the count
 * read during the initialize routine. If invalid, the number
 * of waypoints to read is set to 0. 
 *
 * @param[in] reader  Pointer to reader struct with valid flag, count, EEPROM pointer
 *
 * @returns    Number of waypoints if valid, 0 if invalid
 *
 */
uint32_t waypoint_reader_count(waypoint_reader_t *reader)
{
    return reader->valid ? reader->count : 0;
}

/*!
 * @brief Gets the next waypoint from EEPROM
 *
 * Reads the next waypoint from store by casting lat/long pair
 * to a point typedef struct. Reader state is modified so that 
 * repeated calls return successive waypoints. This call does 
 * not check its own bounds, that responsibility is the user's 
 * (using end).
 *
 * @param[in] reader  Pointer to reader struct with valid flag, count, EEPROM pointer
 *
 * @returns    A point typedef struct containing a lat/long pair read from EEPROM
 *
 */
point_t waypoint_reader_get_next(waypoint_reader_t *reader)
{
    float latitude = eeprom_read_float(reader->ptr++);
    float longitude = eeprom_read_float(reader->ptr++);
    return (point_t){latitude, longitude};
}


/*!
 * @brief Checks if all points read from EEPROM
 *
 * Checks the pointer in the reader struct to see
 * if all waypoints have been read from EEPROM. 
 *
 * @param[in] reader  Pointer to reader struct with valid flag, count, EEPROM pointer
 *
 * @returns    Returns true iff there are no more waypoints to be read
 *
 */
boolean waypoint_reader_end(waypoint_reader_t *reader)
{
    return reader->ptr == (float*)(0x4 + waypoint_reader_count(reader)*8);
}
