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
 * This file contains the function definitions used to write
 * waypoints to EEPROM.
 * 
 * Waypoints are stored in EEPROM as follows:
 *
 *   Count is the number of (latitude, longitude) pairs.
 *
 *   For count = n where n <= MAX_WAYPOINTS
 *
 *   0x00 valid (1 byte)
 *   0x01 n (count) (1 byte)
 *   0x04 1st Waypoint Latitude (4 bytes)
 *   0x08 1st Waypoint Longitude (4 bytes)
 *   0x0C 2nd Waypoint Latitude (4 bytes)
 *   0x10 2nd Waypoint Longitude (4 bytes)
 *   ...
 *   0x?? nth Waypoint Latitude (4 bytes)
 *   0x?? nth Waypoint Longitude (4 bytes)
 *
 */

#include <avr/eeprom.h>
#include <stdint.h>
#include <stdlib.h>

#include "waypoint_writer.h"

#define INVALID 0         /* Waypoints invalid flag */
#define VALID 1           /* All waypoints valid flag */

#define MAX_WAYPOINTS 50  /* Max waypoints allowed */


/*!
 * @brief Initializes writing of waypoints to EEPROM
 *
 * Sets the field being written to EEPROM to the 
 * number of waypoints expected
 *
 * @param[in,out] writer  Pointer to writer struct to initialize
 *
 * @returns    Nothing.
 *
 */
void waypoint_writer_initialize(waypoint_writer_t *writer)
{
    /* Make the first field to write be the count */
    writer->field = COUNT;
}


/*!
 * @brief Writes the value in field to EEPROM
 *
 * Converts field argument to the appropriate value
 * and inserts it into the next address in EEPROM. 
 * Sets the valid flag to INVALID or VALID based on
 * arguements received.
 *
 * @param[in,out] writer  Pointer to a writer typedef struct 
 * @param[in]     field   Field expected to write
 *
 * @returns    A status flag indicating completion of write
 *             for all waypoints, failure to write all 
 *             waypoints, or writing still in process
 *
 */
waypoint_writer_status_t
waypoint_writer_write(waypoint_writer_t *writer, char *field)
{
    uint8_t count;              /* Number of waypoints */
    float latitude, longitude;  /* Latitude/Longitude of waypoint */

    switch (writer->field) {
    case COUNT:
        /* Cast the count to an 8-bit int */
        count = (uint8_t)atoi(field);

        /* If about to receive more than possible, indicate failure */
        if (count > MAX_WAYPOINTS) {
            return FAILURE;
        }
        
        /* Write count to 0x1, then invalid flag.
         * It is assumed invalid until points written
         * == points expected */
        eeprom_write_byte((uint8_t*)0x0, INVALID);
        eeprom_write_byte((uint8_t*)0x1, count);

        /* Move to first waypoint position */
        writer->count = count;
        writer->ptr = (float*)0x4;
        writer->field = LATITUDE;
        break;
    case LATITUDE:
        /* Receive and store latitude, tell struct next value to write is lon */
        latitude = atof(field);
        eeprom_write_float(writer->ptr, latitude);
        writer->ptr++;
        writer->field = LONGITUDE;
        break;
    case LONGITUDE:
        /* Receive and store longitude, tell struct next value to write is lat */
        longitude = atof(field);
        eeprom_write_float(writer->ptr, longitude);
        writer->ptr++;
        writer->field = LATITUDE;
        /* Next waypoint */
        break;
    }

    /* Return a status based on how many points have been written */
    uint8_t written = ((uint32_t)writer->ptr - 0x4)/8;
    if (written == writer->count) {
        /* Written all we expected, write valid flag and return success flag */
        eeprom_write_byte((uint8_t*)0x0, VALID);
        return SUCCESS;
    } else if (written > writer->count) {
        /* Wrote more than expected, indicate failure */
        return FAILURE;
    } else {
        /* There are still more to write, continue working */
        return IN_PROGRESS;
    }
}
