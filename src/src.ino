#include <SPI.h>
#include <avr/eeprom.h>

/* bluetooth dependencies */
#include <EEPROM.h>
#include <lib_aci.h>

#include "bluetooth.h"
#include "lcd.h"
#include "types.h"
#include "haversine.h"
#include "waypoint_reader.h"
#include "waypoint_writer.h"
#include "gps.h"

#define WAYPOINT_DISTANCE_THRESHOLD 100 /* Distance before changing waypoint to next waypoint */
#define BUSY_LED 17                    /* Fio Pin for BUSY LED */

#define GREEN_BUTTON_INTERRUPT_NUM 1  /* Corresponds to pin 2 (D2) */
#define BLUE_BUTTON_INTERRUPT_NUM 0   /* Corresponds to pin 3 (D3) */

/*
 * Global flags indicating a button press has not yet been handled.
 * Will be set to 1 on button press, and should be read and cleared
 * atomically (disable/renable interrupts).
 */
uint8_t g_green_button_pressed = 0;
uint8_t g_blue_button_pressed = 0;

void print_tracking_display(struct tracking_data_t *data)
{
    /* instaneous speed */
    lcd_pos(0, 0);
    lcd_print_str("SP ") ;
    lcd_print_float(data->instant_speed, 1);

    /* average speed */
    lcd_pos(0, 1);
    lcd_print_str("AV ") ;
    lcd_print_float(data->average_speed, 1);

    /* time */
    lcd_pos(0, 2);
    int elapsed = data->time_elapsed;
    int hours = (elapsed/60/60) % 60;
    int minutes = (elapsed/60) % 60;
    int secs = elapsed % 60;
    lcd_print_str("TE ") ;
    lcd_print_time(hours, minutes, secs);

    /* distance */
    lcd_pos(0, 3);
    lcd_print_str("DI ") ;
    lcd_print_float(data->total_distance, 0);

    /* waypoint distance */
    lcd_pos(0, 4);
    lcd_print_str("WP ") ;
    if (data->waypoint_done) {
        lcd_print_str("Done");
    } else {
        lcd_print_float(data->waypoint_distance, 0);
    }
}

void print_home(void)
{
    lcd_clear_display();
    lcd_print_str("CREAM");

    /* print number of waypoints on second row */
    lcd_pos(0, 1);
    waypoint_reader_t waypoint_reader;
    waypoint_reader_initialize(&waypoint_reader);
    uint32_t count = waypoint_reader_count(&waypoint_reader);

    char buffer[13];
    sprintf(buffer, "%d WPs", count);
    lcd_print_str(buffer);
}

void setup(void)
{
    gps_boot();

    /* bluetooth and lcd use SPI, so initialization must be done first */
    SPI.begin();

    bluetooth_t bluetooth;
    bluetooth_setup(&bluetooth);

    /* magic between bluetooth and lcd */
    delay(1);

    /* Initialise LCD - Print startup message */
    lcd_init();
    print_home();

    attachInterrupt(GREEN_BUTTON_INTERRUPT_NUM, green_button_handler, FALLING);
    attachInterrupt(BLUE_BUTTON_INTERRUPT_NUM, blue_button_handler, FALLING);

    pinMode(BUSY_LED, OUTPUT);

    while (1) {
        noInterrupts();

        /* green button press in this context means enter tracking mode */
        if (g_green_button_pressed) {
            g_green_button_pressed = 0;
            interrupts();

            run_tracking();
            gps_standby();

            /* clear all blue presses that occured while in tracking */
            noInterrupts();
            g_blue_button_pressed = 0;
            interrupts();

            print_home();
        }

        /* blue button press in this context means enter bluetooth mode */
        if (g_blue_button_pressed) {
            g_blue_button_pressed = 0;
            interrupts();

            run_bluetooth(&bluetooth);

            /* clear all green presses that occured while in bluetooth */
            noInterrupts();
            g_green_button_pressed = 0;
            interrupts();

            print_home();
        }

        interrupts();
    }
}


void run_tracking(void)
{
    gps_t gps;
    gps_data_t gps_data;
    gps_initialize(&gps);

    waypoint_reader_t waypoint_reader;
    waypoint_reader_initialize(&waypoint_reader);
    point_t initial_waypoint = waypoint_reader_get_next(&waypoint_reader);

    tracking_data_t tracking_data = (tracking_data_t){0.0,0,0.0,0.0,0.0,false};

    tracking_record_t tracking_record = {.num_points = 0,
                                         .aggregate_speed = 0.0,
                                         .current_waypoint = initial_waypoint};

    /* Started will be set to true after the first valid gps packet.
       This indicates that tracking has begun.*/
    boolean started = false;

    lcd_clear_display();
    lcd_print_str("Pending Fix");

    while (1) {

        /* on green button press, finish tracking (return) */
        noInterrupts();
        if (g_green_button_pressed) {
            g_green_button_pressed = 0;
            interrupts();
            return;
        }
        interrupts();

        /* spin until a gps packet has arrived */
        if (gps_available(&gps)) {

            /* ignore invalid packets */
            if (gps_valid(&gps)) {

                /* only clear lcd for the first gps packet after fix */
                if (!started) {
                    lcd_clear_display();
                    started = true;
                }

                gps_parse(&gps, &gps_data);

                update_tracking_record(&tracking_record, &gps_data);
                update_tracking_data(&tracking_data, &gps_data, &tracking_record);

                if (!tracking_data.waypoint_done) {
                    update_waypoint(&waypoint_reader, &tracking_data, &tracking_record);
                }

            }

            /* only update time and display after fix */
            if (started) {
                print_tracking_display(&tracking_data);
                tracking_data.time_elapsed++;
            }
        }
    }
}

void update_tracking_record(tracking_record_t *record, gps_data_t *gps)
{
    record->previous_tracking_point = record->current_tracking_point;
    record->current_tracking_point = gps->location;
    record->num_points++;
    record->aggregate_speed += gps->speed;
}

void update_tracking_data(tracking_data_t *data, gps_data_t *gps, tracking_record_t *record)
{
    /* only add distance if for points 2..n */
    if (record->num_points > 1) {
        data->total_distance +=
            distance_between(record->previous_tracking_point,
                             record->current_tracking_point);
    }
    data->instant_speed = gps->speed;
    data->average_speed = record->aggregate_speed/record->num_points;
}

/* updates tracking_data->waypoint_done, current_waypoint and
 * tracking_data->waypoint_distance */
void update_waypoint(waypoint_reader_t *waypoint_reader,
                     tracking_data_t *tracking_data,
                     tracking_record_t *tracking_record)
{
    point_t waypoint = tracking_record->current_waypoint;
    point_t tracking_point = tracking_record->current_tracking_point;

    float distance_to_waypoint = distance_between(waypoint, tracking_point);

    while (distance_to_waypoint < WAYPOINT_DISTANCE_THRESHOLD
           && !tracking_data->waypoint_done) {
        if (waypoint_reader_end(waypoint_reader)) {
            tracking_data->waypoint_done = true;
        } else {
            waypoint = waypoint_reader_get_next(waypoint_reader);
            distance_to_waypoint = distance_between(waypoint, tracking_point);
        }
    }

    tracking_record->current_waypoint = waypoint;
    tracking_data->waypoint_distance = distance_to_waypoint;
}


/* Never called.
   Loop is handled in setup to avoid using global variables.
   Arduino requires this to compile. */
void loop(void) {}

void green_button_handler(void)
{
    /* only allow button press every 500 ms */
    static uint32_t last = 0;
    uint32_t current = millis();
    if (current - last > 500) {
        g_green_button_pressed = 1;
    }
    last = current;
}

void blue_button_handler(void)
{
    /* only allow button press every 500 ms */
    static uint32_t last = 0;
    uint32_t current = millis();
    if (current - last > 500) {
        g_blue_button_pressed = 1;
    }
    last = current;
}

void run_bluetooth(bluetooth_t *bluetooth)
{
    lcd_clear_display();
    lcd_print_str("Bluetooth");

    /* start advertising and enter recieve routine */
    bluetooth_advertise(bluetooth);
    boolean success = get_and_store_waypoints(bluetooth);

    /* sleep on completion */
    bluetooth_sleep(bluetooth);

    /* show user transfer result */
    char *feedback = (char*)(success ? "Success" : "Failure");
    lcd_clear_display();
    lcd_print_str(feedback);

    /* wait for user to press blue button before returning */
    while (1) {
        noInterrupts();
        if (g_blue_button_pressed) {
            g_blue_button_pressed = 0;
            interrupts();
            return;
        }
        interrupts();
    }
}

/* Recieves a list of waypoints from the bluetooth module and 
   stores them in non-volatile storage.
   Returns true if a list of waypoints was
   completely received and stored correctly.
   Returns false otherwise.
   This routine will block until the transfer is finished or the
   user presses the blue button. */
boolean get_and_store_waypoints(bluetooth_t *bluetooth)
{
    uint8_t started = 0;
    boolean result = false;
    bluetooth_status_t status = bluetooth_get_status(bluetooth);
    waypoint_writer_t waypoint_writer;
    waypoint_writer_initialize(&waypoint_writer);

    while (1) {

        /* quit on blue button pressed */
        noInterrupts();
        if (g_blue_button_pressed) {
            g_blue_button_pressed = 0;
            interrupts();
            return false;
        }
        interrupts();

        bluetooth_poll(bluetooth);
        status = bluetooth_get_status(bluetooth);

        if (bluetooth_has_message(bluetooth)) {
            waypoint_writer_status_t status;
            status = waypoint_writer_write(&waypoint_writer, bluetooth_get_message(bluetooth));
            if (status == SUCCESS) {
                return true;
            } else if (status == FAILURE) {
                return false;
            }
        } else if (STANDBY == status) {
            /* device returns to standby after disconnect */
            return false;
        } else if (CONNECTED == status || ADVERTISING == status) {
            /* keep waiting for message or connection */
            continue;
        } else {
            return false;
        }
    }
}
