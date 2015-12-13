/*!
 * @file
 *
 * @brief Interface for the Adafruit Ultimate GPS
 *
 * @author Andrew Hayford
 * @author Sebastian Luy
 *
 * @date 12 December, 2015
 *
 * This file contains the routines required to interface with
 * the Adafruit Ultimate GPS. 
 * 
 * Based off of the Adafruit/Arduino driver at:
 *    https://github.com/adafruit/Adafruit-GPS-Library
 */

#include "Arduino.h"
#include "gps.h"

/* Macro to reference Serial interface as GPSSerial */
#define GPSSerial Serial1

/* Formatting packets for GPS - sets output type and update frequency */

/* based off of Adafruit arduino driver
   https://github.com/adafruit/Adafruit-GPS-Library */

/* Sample nmea line
   $GPRMC,064951.000,A,2307.1256,N,12016.4438,E,0.03,165.48,260406,3.05,W,A*2C */

/* Datasheet:
   http://www.adafruit.com/datasheets/GlobalTop-FGPMMOPA6H-Datasheet-V0A.pdf */

/* Command Set Datasheet:
   http://www.adafruit.com/datasheets/PMTK_A11.pdf */

/* Formatting packets for GPS - sets output type and update frequency */
#define PMTK_SET_NMEA_OUTPUT_RMCONLY "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
#define PMTK_SET_NMEA_UPDATE_1HZ "$PMTK220,1000*1F"

/* Wakeup and standby packets */
#define PMTK_Q_RELEASE "$PMTK605*31"
#define PMTK_STANDBY "$PMTK161,0*28"

/* Index offsets into a valid NMEA buffer */
#define LATITUDE_OFFSET 20  
#define NS_OFFSET 30
#define LONGITUDE_OFFSET 32
#define EW_OFFSET 43
#define SPEED_OFFSET 45

#define KNOTS_TO_MPH 1.150779

/*!
 * @brief Ignores a line of data from the GPS
 *
 * This routine simply ignores a line from the GPS. Used for ignoring
 * redundant notifications during startup process
 *
 * @returns    Nothing.
 *
 */
static void ignore_line(void) {
    /* DANGEROUS */
    while (!GPSSerial.available() || GPSSerial.read() != '\n');
}

/*!
 * @brief Parses single hex character into decimal
 *
 * This function implements a simple algorithm to convert hexadecimal
 * characters into decimal integers.
 *
 * @param[in]  c    Hexadecimal character (0-9, A-F) 
 *
 * @returns    Decimal equivalent of hexadecimal value
 *
 */
static uint8_t parse_hex(char c) {
    if (c <= '9') {
        return c - '0';
    } else {
        return c - 'A' + 10;
    }
}

/*!
 * @brief Validates the checksum of a GPS datastring
 *
 * This function checks to ensure that the GPS datastring
 * checksum is valid.
 *
 * @param[in]  nmea  Pointer to buffer with NMEA datastring 
 *
 * @returns    True (1) is checksum valid, 0 otherwise
 *
 */
static boolean checksum_good(char *nmea) {
    uint8_t i = 1;
    char c = nmea[1];
    char checksum = 0;

    /* checksum is computed by xor all bytes between $ and * */
    while (c != '*') {
        checksum ^= c;
        c = nmea[++i];
    }

    /* skip '*' and compare with checksum */
    checksum ^= parse_hex(nmea[i + 1])*16 + parse_hex(nmea[i + 2]);

    return checksum == 0;
}

/*!
 * @brief Checks the validity of a GPS datastring with valid flag
 *
 * This function implements a simplistic algorithm to check the valid
 * flag on a GPS datastring
 *
 * @param[in]  nmea    Pointer to a buffer containing a NMEA datastring 
 *
 * @returns    True (1) if flag is valid, 0 otherwise.
 *
 */
static boolean data_valid(char *nmea) {
    char *iter = nmea;
    /* field after second comma */
    iter = strchr(iter, ',');
    if (iter == NULL) return 0;
    iter = strchr(iter + 1, ',');
    if (iter == NULL) return 0;
    /* A means valid */
    return iter[1] == 'A';
}

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
boolean gps_valid(gps_t *gps)
{
    return checksum_good(gps->nmea) && data_valid(gps->nmea);
}

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
void gps_parse(gps_t *gps, gps_data_t *data)
{
    char buffer[7];

    /* latitude */
    char *latitude_field = gps->nmea + LATITUDE_OFFSET;

    /* parse degrees */
    strncpy(buffer, latitude_field, 2);
    buffer[2] = '\0';
    int degrees = atoi(buffer);

    /* parse minutes */
    strncpy(buffer, latitude_field + 2, 2);
    strncpy(buffer + 2, latitude_field + 5, 4);
    buffer[6] = '\0';
    long minutes = atol(buffer);

    /* convert from GGPGA to decimal degrees */
    float latitude = degrees + minutes/600000.0;

    /* parse N/S */
    if (gps->nmea[NS_OFFSET] == 'S') {
        latitude = -latitude;
    }

    data->location.latitude = latitude;

    /* longitude */
    char *longitude_field = gps->nmea + LONGITUDE_OFFSET;

    /* parse degrees */
    strncpy(buffer, longitude_field, 3);
    buffer[3] = '\0';
    degrees = atoi(buffer);

    /* parse minutes */
    strncpy(buffer, longitude_field + 3, 2);
    strncpy(buffer + 2, longitude_field + 6, 4);
    buffer[6] = '\0';
    minutes = atol(buffer);

    /* convert from GGPGA to decimal degrees */
    float longitude = degrees + minutes/600000.0;

    /* parse E/W */
    if (gps->nmea[EW_OFFSET] == 'W') {
        longitude = -longitude;
    }

    data->location.longitude = longitude;

    /* speed */
    char *speed_field = gps->nmea + SPEED_OFFSET;
    strncpy(buffer, speed_field, 4);
    buffer[4] = '\0';

    /* convert from knots to mph */
    data->speed = atof(buffer)*KNOTS_TO_MPH;
}

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
boolean gps_available(gps_t *gps)
{
    while (GPSSerial.available()) {
        char c = GPSSerial.read();
        /* return on new line with valid gps nmea string */
        if (c == '\n') {
            gps->nmea[gps->index] = '\0';
            gps->index = 0;
            /* if more is available, throwout line and read in next */
            if (GPSSerial.available()) {
                continue;
            } else {
                return true;
            }
        /* ignore carriage return */
        } else if (c == '\r') {
            continue;
        } else {
            /* if more than 80 characters read without newline,
               drop remaining */
            if (gps->index < 80) {
                gps->nmea[gps->index++] = c;
            }
        }
    }
    return false;
}

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
void gps_initialize(gps_t *gps)
{
    gps->index = 0;

    /* clear any available data */
    while (GPSSerial.available()) {
        GPSSerial.read();
    }

    /* send message to wakeup gps */
    GPSSerial.println(PMTK_Q_RELEASE);
    ignore_line(); /* startup notification */
    ignore_line(); /* EPO? notification */

    GPSSerial.println(PMTK_SET_NMEA_OUTPUT_RMCONLY);
    ignore_line();

    GPSSerial.println(PMTK_SET_NMEA_UPDATE_1HZ);
    ignore_line();
}

/*!
 * @brief Puts the Adafruit Ultimate GPS in standby mode 
 *
 * This function sends a serial packet to put the Adafruit Ultimate GPS
 * into standby (low power state). 
 *
 * @returns    Nothing.
 *
 */
void gps_standby(void)
{
    GPSSerial.println(PMTK_STANDBY);
}

/*!
 * @brief Starts the Adafruit Ultimate GPS 
 *
 * This function sets up the required parameters to run the GPS. It
 * does this by setting up the serial connection, waking up the GPS,
 * then placing it in standby mode (as to wait until it's actually needed)
 *
 * @returns    Nothing.
 *
 */
void gps_boot(void)
{
    GPSSerial.begin(9600);
    /* send any message to wakeup if in standby already */
    GPSSerial.println(PMTK_Q_RELEASE);
    ignore_line();
    gps_standby();
}
