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
    int elapsed = data->time_elapsed;
    int hours = (elapsed/60/60) % 60;
    int minutes = (elapsed/60) % 60;
    int secs = elapsed % 60;
    lcd_print_time(hours, minutes, secs);

    lcd_pos(0, 3);
    lcd_print_float(data->total_distance);

    lcd_pos(0, 4);
    if (data->waypoint_done) {
        lcd_write_str("Waypoint Complete");
    } else {
        lcd_print_float(data->waypoint_distance);
    }
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

    point_t initial_waypoint = waypoint_store_get_next(&waypoint_store);

    tracking_data_t tracking_data = (tracking_data_t){0.0,0,0.0,0.0,0.0,0};
    tracking_record_t tracking_record = {.num_points = 0,
                                         .aggregate_speed = 0.0,
                                         .current_waypoint = initial_waypoint};
    int received = 0;

    while (1) {

        if (gps_available(&gps)) {
            if (gps_valid(&gps)) {
                digitalWrite(BUSY_LED, HIGH); /* turn on LED */

                gps_parse(&gps, &gps_data);

                tracking_record.previous_tracking_point = tracking_record.current_tracking_point;
                tracking_record.current_tracking_point = gps_data.location;
                tracking_record.num_points++;
                tracking_record.aggregate_speed += gps_data.speed;

                /* only add distance if for points 2..n */
                if (tracking_record.num_points > 1) {
                    tracking_data.total_distance +=
                        distance_between(tracking_record.previous_tracking_point,
                                         tracking_record.current_tracking_point);
                }

                tracking_data.instant_speed = gps_data.speed;
                tracking_data.average_speed =
                    tracking_record.aggregate_speed/tracking_record.num_points;

                update_waypoint(&waypoint_store, &tracking_data, &tracking_record);

                digitalWrite(BUSY_LED, LOW); /* turn off LED */
            }
            print_tracking_display(&tracking_data);
            tracking_data.time_elapsed++;
        }
    }
}

/* updates tracking_data->waypoint_done, current_waypoint and
 * tracking_data->waypoint_distance */
void update_waypoint(waypoint_store_t *waypoint_store,
                     tracking_data_t *tracking_data,
                     tracking_record_t *tracking_record)
{
    if (!tracking_data->waypoint_done) {

        point_t waypoint = tracking_record->current_waypoint;
        point_t tracking_point = tracking_record->current_tracking_point;

        float distance_to_waypoint = distance_between(waypoint, tracking_point);

        while (distance_to_waypoint < WAYPOINT_DISTANCE_THRESHOLD
               && !tracking_data->waypoint_done) {
            if (waypoint_store_end(waypoint_store)) {
                tracking_data->waypoint_done = 1;
            } else {
                waypoint = waypoint_store_get_next(waypoint_store);
                distance_to_waypoint = distance_between(waypoint, tracking_point);
            }
        }

        tracking_record->current_waypoint = waypoint;
        tracking_data->waypoint_distance = distance_to_waypoint;
    }
}


/* this loop/setup is begging for globals */
void loop(void)
{ 
}
