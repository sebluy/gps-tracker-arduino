/*!
 * @file
 *
 * @brief Main file with user-input related functions
 *
 * @author Andrew Hayford
 * @author Sebastian Luy
 *
 * @date 12 December, 2015
 *
 * This is the main file for the CREAM project. Functions
 * included in this file handle the device status tracking/
 * printing and functions for handling user input accordingly
 * 
 */

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
#define BUSY_LED 17                     /* Fio Pin for BUSY LED */

#define GREEN_BUTTON_INTERRUPT_NUM 1  /* Corresponds to pin 2 (D2) */
#define BLUE_BUTTON_INTERRUPT_NUM 0   /* Corresponds to pin 3 (D3) */

/*
 * Global flags indicating a button press has not yet been handled.
 * Will be set to 1 on button press, and should be read and cleared
 * atomically (disable/renable interrupts).
 */
uint8_t g_green_button_pressed = 0;
uint8_t g_blue_button_pressed = 0;

/*!
 * @brief Interrupt handler for green button (tracking)
 *
 * This is the interrupt handler for the green button. Upon 
 * pressing the button, the tracking mode flag is set, and the
 * device will enter the tracking mode routines. Button presses 
 * are only received if they are 500ms apart.
 *
 * @returns    Nothing.
 *
 */
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

/*!
 * @brief Interrupt handler for blue button (bluetooth)
 *
 * This is the interrupt handler for the blue button. Upon 
 * pressing the button, the Bluetooth mode flag is set and
 * the device will enter Bluetooth mode. Button presses are 
 * only received if they are 500ms apart.
 *
 * @returns    Nothing.
 *
 */
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

/*!
 * @brief Setup function - required by Arduino
 *
 * This is another Arduino-required function. All
 * the peripherals and interrupts are setup here.
 * Additionally, the main loop is handled in this
 * function instead of loop() in order to avoid
 * global variables. Main loop sends us either to
 * home, tracking, or bluetooth modes based on 
 * flags set by interrupt handlers
 *
 * @returns    Nothing.
 *
 */
void setup(void)
{
    gps_boot(); /* start gps */

    /* bluetooth and lcd use SPI, so initialization must be done first */
    SPI.begin();

    /* initialise the bluetooth */
    bluetooth_t bluetooth;
    bluetooth_setup(&bluetooth);

    /* magic between bluetooth and lcd */
    delay(1);

    /* Initialise LCD - Print startup message */
    lcd_init();
    print_home();

    /* Attach interrupts to pins with buttons connected */
    attachInterrupt(GREEN_BUTTON_INTERRUPT_NUM, green_button_handler, FALLING);
    attachInterrupt(BLUE_BUTTON_INTERRUPT_NUM, blue_button_handler, FALLING);

    /* Setup LED for debugging */
    pinMode(BUSY_LED, OUTPUT);

    /* main loop */
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

/*!
 * @brief Arduino-required function - never called
 *
 * Arduino requires this to compile. Loop is handled in setup 
 * to avoid using global variables.
 *
 * @returns    Nothing.
 *
 */
void loop(void) {}

/*!
 * @brief Prints tracking details while in tracking mode
 *
 * Handles the user interface for tracking mode. Prints 
 * instantaneous speed, average speed, time elapsed, distance
 * traveled, and distance to current waypoint, each prepended
 * with a two character indicator of the value
 *
 * @param[in] data  Pointer to current tracking data to display
 *
 * @returns    Nothing.
 *
 */
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

/*! @brief Prints the device name and number of waypoints
 *
 * Prints the main screen when not in tracking or bluetooth
 * modes. Lists number of waypoint and displays project title.
 *
 * @returns    Nothing.
 *
 */
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


/*!
 * @brief Runs tracking mode
 *
 * This function runs tracking mode (entered when green button
 * is pressed). Tracking mode is implemented by waiting for a 
 * gps packet. Once a packet is received and validated, it is 
 * parsed and the resulting data is fed into the tracking data
 * structs
 *
 * @returns    Nothing.
 *
 */
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

/*!
 * @brief Updates record for average speed/previous point tracking
 *
 * Updates the tracking record struct with the previous and current points,
 * and adds the current speed to the aggregate speed (used to calculate the
 * average speed).
 *
 * @param[in,out] record  Pointer to tracking record to update
 * @param[in]     gps     Pointer to gps struct with received datastring
 *
 * @returns    Nothing.
 *
 */
void update_tracking_record(tracking_record_t *record, gps_data_t *gps)
{
    record->previous_tracking_point = record->current_tracking_point;
    record->current_tracking_point = gps->location;
    record->num_points++;
    record->aggregate_speed += gps->speed;
}

/*!
 * @brief Updates record for average speed/previous point tracking
 *
 * Updates the tracking record struct with the previous and current points,
 * and adds the current speed to the aggregate speed (used to calculate the
 * average speed).
 *
 * @param[in,out] data    Pointer to data struct for current tracking cycle
 * @param[in]     gps     Pointer to gps struct with received datastring
 * @param[in]     record  Pointer to updated record for current gps data set
 *
 * @returns    Nothing.
 *
 */
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

/*!
 * @brief Updates waypoint information
 *
 * Updates tracking_data->waypoint_done, current_waypoint and
 * tracking_data->waypoint_distance for current set of data read 
 * from the gps. Tells us the distance from the current waypoint, 
 * switches to next waypoint if needed, or ends the path if all
 * waypoints have been passed.
 *
 * @param[in]      waypoint_reader   Pointer to waypoint_reader for current tracking session
 * @param[in,out]  data              Pointer to tracking data struct to update
 * @param[in,out]  record            Pointer to tracking record struct to update
 *
 * @returns    Nothing.
 *
 */
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

/*!
 * @brief Runs the Bluetooth mode after blue button is pressed
 *
 * This function handles the Bluetooth mode after the user presses
 * the blue button on the device. This mode tells the Bluetooth module
 * to advertise. Once a connection is made, we wait for messages
 * (containing waypoint data) and write them to storage. The result of
 * the transaction (success or failure) is displayed on the LCD.
 *
 * @param[in,out]  bluetooth  Pointer to bluetooth struct
 *
 * @returns    Nothing.
 *
 */
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

/*!
 * @brief Gets waypoints over bluetooth and puts them in storage
 *
 * Recieves a list of waypoints from the bluetooth module and 
 * stores them in non-volatile storage.Returns true if a list 
 * of waypoints was completely received and stored correctly.
 * Returns false otherwise. This routine will block until the 
 * transfer is finished or the user presses the blue button. 
 *
 * @param[in,out]  bluetooth  Pointer to bluetooth struct
 *
 * @returns    True if complete list of waypoints received and
 *             stored, false otherwise.
 *
 */
boolean get_and_store_waypoints(bluetooth_t *bluetooth)
{
    uint8_t started = 0;
    boolean result = false;
    bluetooth_status_t status = bluetooth_get_status(bluetooth);

    /* Setup waypoint writer struct */
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

        /* Update bluetooth status and check*/
        bluetooth_poll(bluetooth);
        status = bluetooth_get_status(bluetooth);

        /* If a message is available, we want to write it
         * to EEPROM. If not, we wait for a connection or
         * a message.
         */
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
