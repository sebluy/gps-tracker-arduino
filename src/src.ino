#include <SPI.h>
#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>

#include "nokia_5110.h"

extern "C" {
#include "types.h"
#include "haversine.h"
#include "waypoint_store.h"
}

#define GPSSerial Serial1

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
    waypoint_store_t waypoint_store;
    point_t waypoint;
    point_t tracking_point;
    char c;
    float distance_to_waypoint;
    int waypoint_num = 0;

    waypoint_store_initialize(&waypoint_store);
    waypoint = waypoint_store_get_next(&waypoint_store);

    while (1) {
        c = GPS.read();
        if (GPS.newNMEAreceived() && GPS.parse(GPS.lastNMEA()) && GPS.fix) {
            
            digitalWrite(BUSY_LED, HIGH); /* turn on LED */

            tracking_point.latitude = GPS.latitudeDegrees;
            tracking_point.longitude = GPS.longitudeDegrees;
            
            distance_to_waypoint = distance_between(waypoint, tracking_point);

            if (distance_to_waypoint < WAYPOINT_DISTANCE_THRESHOLD) {
                if (waypoint_store_end(&waypoint_store)) {
                    /* loop forever if out of waypoints */
                    lcd_write_str("Path Complete");
                    while (1);
                } else {
                    /* else move on to next waypoint */
                    waypoint = waypoint_store_get_next(&waypoint_store);
                    distance_to_waypoint = distance_between(waypoint, tracking_point);
                }
            }

            print_distance_to_waypoint(distance_to_waypoint);

            digitalWrite(BUSY_LED, LOW); /* turn off LED */
        }
    }
}

void print_distance_to_waypoint(double distance)
{
  lcd_pos(0,0);
  lcd_print_float(distance);
}
