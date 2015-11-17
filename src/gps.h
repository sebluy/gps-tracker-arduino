/*!
 * @file
 *
 * @brief Header file for interfacing with the Adafruit Ultimate GPS
 *
 * @author Andrew Hayford
 * @author Sebastian Luy
 *
 * @date 11 November, 2015
 *
 * This file contains the data structures and function prototypes 
 * for all routines used to interface with the Adafruit Ultimate GPS.
 * 
 */

#ifndef GPS_H
#define GPS_H

#include <stdint.h>
#include "types.h"

#define NMEA_LINE_LENGTH 80  /*!< Length of NMEA datastrings coming from GPS */

/*!
 * @brief struct to hold raw NMEA datastrings coming from GPS
 *
 * This struct holds the buffer which takes in NMEA datastrings from the GPS,
 * as well as the index into the buffer for parsing and error checking.
 *
 */
struct gps_t {
    char nmea[NMEA_LINE_LENGTH + 1];  /*!< NMEA buffer */
    int index;                        /*!< Index into NMEA buffer */
};

/*!
 * @brief struct to hold the lat/long coordinate and speed for a GPS data packet
 *
 * This struct holds the latitude and longitude pair, as well as the current 
 * speed, for a data packet delivered by the GPS
 *
 */
struct gps_data_t {
    point_t location;
    float speed;
};

/*!
 * @brief Starts the Adafruit Ultimate GPS 
 *
 * This function sets up the required parameters to run the GPS. It
 * does this by setting up the serial connection, waking up the GPS,
 * then placing it in standby mode
 *
 * @returns    Nothing.
 *
 */
void gps_boot(void);

/*!
 * @brief Puts the Adafruit Ultimate GPS in standby mode 
 *
 * This function sends a serial packet to put the Adafruit Ultimate GPS
 * into standby (low power state). 
 *
 * @returns    Nothing.
 *
 */
void gps_standby(void);

/*!
 * @brief Initialises the Adafruit Ultimate GPS
 *
 * This function completes the required initialisation procedures to
 * use the Adafruit Ultimate GPS. This wakes up the GPS, sets the update
 * frequency, and sets up the output packet format.
 * 
 * @param[in,out]  gps  Pointer to uninitialised GPS struct
 *
 * @returns    Nothing.
 *
 */
void gps_initialize(gps_t *gps);

/*!
 * @brief Checks if GPS is available
 *
 * This function checks to see if a new, valid NMEA GPS datastring is available.
 * Only returns true (1) once per datastring
 *
 * @param[in,out]  gps    Pointer to a GPS struct 
 *
 * @returns    1 if a new, valid gps NMEA string is available/ 0 otherwise
 *
 */
uint8_t gps_available(gps_t *gps);

/*!
 * @brief Validates the GPS datastring checksum/valid flag
 *
 * This function implements error checking for receiving GPS datastrings by
 * by validating the checksum and valid flag
 *
 * @param[in]  gps    Pointer to a GPS struct containing an NMEA string
 *
 * @returns    1 if checksum/flag is valid, 0 otherwise
 *
 */
uint8_t gps_valid(gps_t *gps);

/*!
 * @brief Parses an NMEA datastring
 *
 * This function parses an NMEA datastring by reading and translating the buffer 
 * stored in the gps_t struct, and storing the results in a gps_data_t struct 
 * for ease of use
 *
 * @param[in]      gps    Pointer to a GPS struct containing an NMEA string
 * @param[in,out]  data   Pointer to a gps_data struct to store parsed data
 *
 * @returns    Nothing.
 *
 */
void gps_parse(gps_t *gps, gps_data_t *data);

#endif
