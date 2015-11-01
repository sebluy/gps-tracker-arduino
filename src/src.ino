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

void print_tracking_display(struct tracking_data_t *data)
{
    lcd_pos(0, 0);
    lcd_print_float(data->instant_speed);
    lcd_pos(0, 1);
    lcd_print_float(data->average_speed);
    lcd_pos(0, 2);
    lcd_print_float(data->time_elapsed);
    lcd_pos(0, 3);
    lcd_print_float(data->total_distance);
    lcd_pos(0, 4);
    lcd_print_float(data->waypoint_distance);
}

void setup(void)
{
    /* Initialise LCD - Print startup message */
    lcd_init();
    lcd_clear_display();
    lcd_write_str("Waiting for fix...");
  
    pinMode(BUSY_LED, OUTPUT);

    /* Configure GPS - 1Hz update frequency, full NMEA output */
    Adafruit_GPS gps(&GPSSerial);
    gps.begin(9600);
    gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ) ;
    gps.sendCommand(PGCMD_ANTENNA);
    gps.sendCommand(PMTK_Q_RELEASE);

    waypoint_store_t waypoint_store;
    waypoint_store_initialize(&waypoint_store);

    point_t waypoint = waypoint_store_get_next(&waypoint_store);

    tracking_data_t tracking_data = (tracking_data_t){0.0,0.0,0.0,0.0,0.0};
    tracking_record_t tracking_record = {.num_points = 0, .aggregate_speed = 0.0};

    while (1) {

        gps_wait_for_packet(gps);

        digitalWrite(BUSY_LED, HIGH); /* turn on LED */

        point_t tracking_point = (point_t){gps.latitudeDegrees, gps.longitudeDegrees};

        tracking_record.num_points++;
        tracking_record.aggregate_speed += gps.speed;

        /* only add distance if for points 2..n */
        if (tracking_record.num_points > 1) {
            tracking_data.total_distance +=
                distance_between(tracking_record.previous_point, tracking_point);
        }
        tracking_record.previous_point = tracking_point;

        tracking_data.instant_speed = gps.speed;
        tracking_data.average_speed =
            tracking_record.aggregate_speed/tracking_record.num_points;

        float distance_to_waypoint = distance_between(waypoint, tracking_point);

        if (tracking_data.waypoint_distance < WAYPOINT_DISTANCE_THRESHOLD) {
            if (waypoint_store_end(&waypoint_store)) {
                /* loop forever if out of waypoints */
                lcd_write_str("Path Complete");
                while (1);
            } else {
                /* else move on to next waypoint */
                waypoint = waypoint_store_get_next(&waypoint_store);
                distance_to_waypoint = distance_between(waypoint, tracking_point);
                lcd_write_str("Moving Waypoint");
            }
        }

        tracking_data.waypoint_distance = distance_to_waypoint;

        print_tracking_display(&tracking_data);

        digitalWrite(BUSY_LED, LOW); /* turn off LED */
    }

}

/* this loop/setup is begging for globals */
void loop(void)
{ 
}

void gps_wait_for_packet(Adafruit_GPS& gps)
{
    /* wait until a packet arrives with a fix */
    while (!(gps.newNMEAreceived() && gps.parse(gps.lastNMEA()) && gps.fix)) {
        gps.read();
    }
}

