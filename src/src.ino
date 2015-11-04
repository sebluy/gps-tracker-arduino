#include <SPI.h>

#include "nokia_5110.h"
#include "time.h"
#include "types.h"
#include "haversine.h"
#include "waypoint_store.h"
#include "gps.h"

#define WAYPOINT_DISTANCE_THRESHOLD 50 /* meters */
#define BUSY_LED 17

void print_tracking_display(struct tracking_data_t *data)
{
    lcd_clear_display();
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

    gps_t gps;
    gps_data_t gps_data;
    /* Configure GPS - 1Hz update frequency, full NMEA output */
    gps_initialize(&gps);

    waypoint_store_t waypoint_store;
    waypoint_store_initialize(&waypoint_store);

    point_t waypoint = waypoint_store_get_next(&waypoint_store);

    tracking_data_t tracking_data = (tracking_data_t){0.0,0.0,0.0,0.0,0.0};
    tracking_record_t tracking_record = {.num_points = 0, .aggregate_speed = 0.0};

    while (1) {

        if (gps_available(&gps)) {
            digitalWrite(BUSY_LED, HIGH); /* turn on LED */

            gps_parse(&gps, &gps_data);

            point_t tracking_point = gps_data.location;

            tracking_record.num_points++;
            tracking_record.aggregate_speed += gps_data.speed;

            tracking_data.instant_speed = gps_data.speed;
            tracking_data.average_speed =
                tracking_record.aggregate_speed/tracking_record.num_points;

            /* only add distance if for points 2..n */
            if (tracking_record.num_points > 1) {
                tracking_data.total_distance +=
                    distance_between(tracking_record.previous_point, tracking_point);
            }

            tracking_record.previous_point = tracking_point;

            float distance_to_waypoint = distance_between(waypoint, tracking_point);

            while (distance_to_waypoint < WAYPOINT_DISTANCE_THRESHOLD) {
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

            tracking_data.waypoint_distance = distance_to_waypoint;

            print_tracking_display(&tracking_data);

            digitalWrite(BUSY_LED, LOW); /* turn off LED */
        }
    }

}

/* this loop/setup is begging for globals */
void loop(void)
{ 
}
