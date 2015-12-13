/*!
 * @file
 *
 * @brief Header file for reading waypoints from EEPROM
 *
 * @author Andrew Hayford
 * @author Sebastian Luy
 *
 * @date 12 December, 2015
 *
 * This file contains the function prototypes  needed to read waypoints
 * from EEPROM. 
 * 
 * See waypoint writer for details on waypoint path layout in memory
 */
#ifndef WAYPOINT_READER_H
#define WAYPOINT_READER_H

#include <stdint.h>
#include "Arduino.h"

#include "types.h"

/*!
 * @brief Struct to hold data for reading waypoints from EEPROM
 *
 * A waypoint reader is used for reading waypoints from non-volatile storage.
   Use a waypoint writer to write waypoints to storage.
 *
 */
struct waypoint_reader_t {
    uint8_t count;
    uint8_t valid;
    float *ptr;
};


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
void waypoint_reader_initialize(waypoint_reader_t *reader);

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
uint32_t waypoint_reader_count(waypoint_reader_t *reader);

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
point_t waypoint_reader_get_next(waypoint_reader_t *reader);

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
boolean waypoint_reader_end(waypoint_reader_t *reader);

#endif
