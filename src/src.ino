#include <SPI.h>
#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>
#include <Wire.h>

#include "nokia_5110.h"

/* include C file */
extern "C" {
#include "haversine.h"
}

#define GPSSerial Serial1          /* Serial talking to GPS */

#define WAYPOINT_DISTANCE_THRESHOLD 50 /* meters */
#define BUSY_LED 17

/* Define Adafruit GPS object */
Adafruit_GPS GPS(&GPSSerial);

void setup(void)
{
  /* Initialise LCD - Print startup message */
  lcd_init();
  lcd_clear_display();
  lcd_write_str("Cache Rules Everything Around Me");
  delay(2500) ;
  lcd_clear_display();
  
  /* Wait for serial to be ready - start GPS at 9600 baud */
  GPS.begin(9600);

  pinMode(BUSY_LED, OUTPUT);

  /* Configure GPS - 1Hz update frequency, full NMEA output */
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ) ;
  GPS.sendCommand(PGCMD_ANTENNA);
  GPSSerial.println(PMTK_Q_RELEASE);
}

void loop(void)
{ 
    float *eeprom_index = 0;
    uint32_t waypoint_count = eeprom_read_dword((uint32_t*)eeprom_index++);
    /* waypoints from eeprom are stored in degrees */
    float waypoint_latitude = eeprom_read_float(eeprom_index++);
    float waypoint_longitude = eeprom_read_float(eeprom_index++);
    float distance_to_waypoint;
    int waypoint_num = 0;

    char c;

    while (1) {
        c = GPS.read();
        if (GPS.newNMEAreceived() && GPS.parse(GPS.lastNMEA()) && GPS.fix) {
            /* mark GPS as read, kind of a hack */
            
            digitalWrite(BUSY_LED, HIGH); /* turn on LED */
            
            distance_to_waypoint =
                distance_between_d(waypoint_latitude, waypoint_longitude,
                                    GPS.latitudeDegrees, GPS.longitudeDegrees);

            if (distance_to_waypoint < WAYPOINT_DISTANCE_THRESHOLD) {
                /* get next waypoint */
                waypoint_latitude = eeprom_read_float(eeprom_index++);
                waypoint_longitude = eeprom_read_float(eeprom_index++);

                /* update waypoint num */
                waypoint_num++;
                waypoint_num = waypoint_num < waypoint_count ? waypoint_num + 1 : 0;

                /* calculate distance to next waypoint */
                distance_to_waypoint =
                    distance_between_d(waypoint_latitude, waypoint_longitude,
                                        GPS.latitudeDegrees, GPS.longitudeDegrees);
            }

            print_distance_to_waypoint(distance_to_waypoint, waypoint_latitude, waypoint_longitude);

            digitalWrite(BUSY_LED, LOW); /* turn off LED */
        }
    }
}

void print_distance_to_waypoint(double distance, double way_lat, double way_lng)
{
  lcd_pos(0,0);
  lcd_print_float(distance);
  lcd_pos(0,1);
  lcd_print_float(way_lat);
  lcd_pos(0,2);
  lcd_print_float(way_lng);
  lcd_pos(0,3);
  lcd_print_float(GPS.latitudeDegrees);
  lcd_pos(0,4);
  lcd_print_float(GPS.longitudeDegrees);
}

