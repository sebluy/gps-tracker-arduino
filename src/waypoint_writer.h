/*!
 * @file
 *
 * @brief Header file for writing waypoints to storage
 *
 * @author Andrew Hayford
 * @author Sebastian Luy
 *
 * @date 12 December, 2015
 *
 * This file contains the function prototypes used to write
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

#ifndef WAYPOINT_WRITER_H
#define WAYPOINT_WRITER_H

#include "Arduino.h"

/*!
 * @brief enum holding acceptable values for fields to write to storage
 *
 * This enum holds all the accepted values for field in the waypoint_writer_t
 * struct. These values include the COUNT (number of waypoints expected), the
 * LATITUDE (lat associated with current waypoint), and LONGITUDE (lon associated
 * with the current waypoint).
 *
 */
enum waypoint_field_t {COUNT, LATITUDE, LONGITUDE};

/*!
 * @brief enum holding possible statuses of write transaction
 *
 * Possible outcomes of a given write transaction. A transaction can fail
 * (such as if there are more points received than expected), succeeded 
 * (points written == points expected) or still be in progress.
 *
 */
enum waypoint_writer_status_t {FAILURE, IN_PROGRESS, SUCCESS};

/*!
 * @brief struct holding bookkeeping values for writing waypoints to EEPROM
 *
 * A waypoint writer is used for storing waypoints in non-volatile 
 * storage. Use a waypoint reader to get waypoints out of storage.
 * Only one waypoint path can be stored at any given time. Creating 
 * a new waypoint_writer and writing to it will overwrite the previous
 * waypoint path.
 *
 */
struct waypoint_writer_t {
    waypoint_field_t field;
    uint32_t count;
    float *ptr;
};

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
void waypoint_writer_initialize(waypoint_writer_t *writer);


/*!
 * @brief Writes the value in field to EEPROM
 *
 * Converts field argument to the appropriate value
 * and inserts it into the next address in EEPROM. 
 * Sets the valid flag to INVALID or VALID based on
 * arguements received.After initialization, should 
 * be called successively with count, lat1, lng1, 
 * lat2, lng2, ... for "count" points. The path will 
 * not be "valid" until "count" points have been written.
 *
 * @param[in,out] writer  Pointer to a writer typedef struct 
 * @param[in]     field   Field expected to write
 *
 * @returns    Current status of the transaction; 
 *             value indicates completion of write
 *             for all waypoints, failure to write all 
 *             waypoints, or writing still in process
 *
 */   
waypoint_writer_status_t
waypoint_writer_write(waypoint_writer_t *writer, char *field);

#endif
