#include "Arduino.h"
#include "gps.h"

/* based off of Adafruit arduino driver
   https://github.com/adafruit/Adafruit-GPS-Library */

#define GPSSerial Serial1

#define PMTK_SET_NMEA_OUTPUT_RMCONLY "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
#define PMTK_SET_NMEA_UPDATE_1HZ "$PMTK220,1000*1F"

#define PMTK_Q_RELEASE "$PMTK605*31"
#define PMTK_STANDBY "$PMTK161,0*28"

#define LATITUDE_OFFSET 20
#define NS_OFFSET 30
#define LONGITUDE_OFFSET 32
#define EW_OFFSET 43
#define SPEED_OFFSET 45

#define KNOTS_TO_MPH 1.150779

static void ignore_line(void) {
    while (!GPSSerial.available() || GPSSerial.read() != '\n');
}

static uint8_t parse_hex(char c) {
    if (c <= '9') {
        return c - '0';
    } else {
        return c - 'A' + 10;
    }
}

static uint8_t checksum_good(char *nmea) {
    uint8_t i = 1;
    char c = nmea[1];
    char checksum = 0;

    while (c != '*') {
        checksum ^= c;
        c = nmea[++i];
    }

    /* skip '*' and compare with checksum */
    checksum ^= parse_hex(nmea[i + 1])*16 + parse_hex(nmea[i + 2]);

    return checksum == 0;
}

static uint8_t data_valid(char *nmea) {
    char *iter = nmea;
    /* field after second comma */
    iter = strchr(iter, ',');
    if (iter == NULL) return 0;
    iter = strchr(iter + 1, ',');
    if (iter == NULL) return 0;
    /* A means valid */
    return (uint8_t)(iter[1] == 'A');
}

uint8_t gps_valid(gps_t *gps)
{
    return checksum_good(gps->nmea) && data_valid(gps->nmea);
}

/* parses string in nmea, filling data fields of gps data */
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

    data->speed = atof(buffer)*KNOTS_TO_MPH;
}


/* returns true if valid gps nmea string is available
 * only returns true once per valid nmea string */
uint8_t gps_available(gps_t *gps)
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
                return 1;
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
    return 0;
}

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

void gps_standby(void)
{
    GPSSerial.println(PMTK_STANDBY);
}

void gps_boot(void)
{
    GPSSerial.begin(9600);
    /* send any message to wakeup if in standby already */
    GPSSerial.println(PMTK_Q_RELEASE);
    ignore_line();
    gps_standby();
}
