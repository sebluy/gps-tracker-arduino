#include <SPI.h>
#include <avr/eeprom.h>

#include "Adafruit_BLE_UART.h"

#include "nokia_5110.h"
#include "time.h"
#include "types.h"
#include "haversine.h"
#include "waypoint_store.h"
#include "gps.h"

#define REQ 9
#define RDY 7
#define RST 10

#define WAYPOINT_DISTANCE_THRESHOLD 50 /* meters */
#define BUSY_LED 17

#define GREEN_BUTTON_INTERRUPT_NUM 1 /* corresponds to pin 2 (D2) */
#define BLUE_BUTTON_INTERRUPT_NUM 0 /* corresponds to pin 3 (D3) */

/* Global flag indicating a button press has not yet been handled.
   Will be set to 1 on button press, and should be read and cleared
   atomically (disable/renable interrupts). */
uint8_t g_green_button_pressed = 0;
uint8_t g_blue_button_pressed = 0;

void print_tracking_display(struct tracking_data_t *data)
{
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

void print_memory_usage()
{
    int a = 0;
    Serial.print("Stack: ");
    Serial.println((int)&a);

    uint8_t *b = (uint8_t*)malloc(1);
    free(b);
    Serial.print("Heap: ");
    Serial.println((int)b);
}

/* todo: standby/active bluetooth and gps
   make bluetooth interruptable
   make bluetooth transactions reliable and "transactional" */
void setup(void)
{
    /* Initialise LCD - Print startup message */
    lcd_init();
    lcd_clear_display();
    lcd_write_str("CREAM");

    attachInterrupt(GREEN_BUTTON_INTERRUPT_NUM, green_button_handler, FALLING);
    attachInterrupt(BLUE_BUTTON_INTERRUPT_NUM, blue_button_handler, FALLING);

    pinMode(BUSY_LED, OUTPUT);

    int tracking;
    while (1) {
        noInterrupts();

        /* handle green button press */
        if (g_green_button_pressed) {
            g_green_button_pressed = 0;
            interrupts();

            run_tracking();

            /* clear all blue presses that occured while in tracking */
            noInterrupts();
            g_blue_button_pressed = 0;
            interrupts();

            lcd_clear_display();
            lcd_write_str("CREAM");
        }

        /* handle blue button press */
        if (g_blue_button_pressed) {
            g_blue_button_pressed = 0;
            interrupts();

            run_bluetooth();

            /* clear all green presses that occured while in bluetooth */
            noInterrupts();
            g_green_button_pressed = 0;
            interrupts();

            lcd_clear_display();
            lcd_write_str("CREAM");
        }

        interrupts();
    }
}


void run_tracking(void)
{
    gps_t gps;
    gps_data_t gps_data;
    gps_initialize(&gps);

    waypoint_store_t waypoint_store;
    waypoint_store_initialize(&waypoint_store);
    point_t initial_waypoint = waypoint_store_get_next(&waypoint_store);

    tracking_data_t tracking_data = (tracking_data_t){0.0,0,0.0,0.0,0.0,0};
    tracking_record_t tracking_record = {.num_points = 0,
                                         .aggregate_speed = 0.0,
                                         .current_waypoint = initial_waypoint};

    int started = 0;
    lcd_clear_display();
    lcd_write_str("Pending Fix");

    while (1) {

        noInterrupts();
        if (g_green_button_pressed) {
            g_green_button_pressed = 0;
            interrupts();
            return;
        }
        interrupts();

        if (gps_available(&gps)) {

            digitalWrite(BUSY_LED, HIGH); /* turn on LED */

            if (gps_valid(&gps)) {

                if (!started) {
                    lcd_clear_display();
                    started = 1;
                }

                gps_parse(&gps, &gps_data);

                update_tracking_record(&tracking_record, &gps_data);
                update_tracking_data(&tracking_data, &gps_data, &tracking_record);

                if (!tracking_data.waypoint_done) {
                    update_waypoint(&waypoint_store, &tracking_data, &tracking_record);
                }

            }

            if (started) {
                print_tracking_display(&tracking_data);
                tracking_data.time_elapsed++;
            }

            digitalWrite(BUSY_LED, LOW); /* turn off LED */
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
void update_waypoint(waypoint_store_t *waypoint_store,
                     tracking_data_t *tracking_data,
                     tracking_record_t *tracking_record)
{
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

/* this loop/setup is begging for globals */
void loop(void)
{ 
}

/* called on button press interrupt */
void green_button_handler(void)
{
    /* only allow button press every 200 ms */
    static uint32_t last = 0;
    uint32_t current = millis();
    if (current - last > 200) {
        g_green_button_pressed = 1;
    }
    last = current;
}

void blue_button_handler(void)
{
    /* only allow button press every 200 ms */
    static uint32_t last = 0;
    uint32_t current = millis();
    if (current - last > 200) {
        g_blue_button_pressed = 1;
    }
    last = current;
}

void run_bluetooth(void)
{
    lcd_clear_display();
    lcd_write_str("Bluetooth");
    Adafruit_BLE_UART bluetooth = Adafruit_BLE_UART(REQ, RDY, RST);
    bluetooth.setDeviceName("PETRICE"); /* 7 characters max! */
    bluetooth.begin();
    get_and_store_waypoints(bluetooth);
}

void get_and_store_waypoints(Adafruit_BLE_UART bluetooth)
{
    char buffer[21];
    uint32_t count, received;
    float latitude, longitude;
    uint32_t *eeprom_address = 0;

    /* get count */
    bluetooth_get_token(bluetooth, buffer);
    count = atoi(buffer);

    eeprom_write_dword(eeprom_address, count);
    eeprom_address += 1;

    /* get latitudes and longitudes */
    for (received = 0; received < count; received++) {

        bluetooth_get_token(bluetooth, buffer);
        latitude = (float)atof(buffer);
        eeprom_write_float((float*)eeprom_address, latitude);

        eeprom_address += 1;

        bluetooth_get_token(bluetooth, buffer);
        longitude = (float)atof(buffer);
        eeprom_write_float((float*)eeprom_address, longitude);

        eeprom_address += 1;
    }
}

/* blocks until data is available from bluetooth */
aci_evt_opcode_t bluetooth_get_token(Adafruit_BLE_UART bluetooth, char buffer[20])
{
    aci_evt_opcode_t status;
    int pos = 0;

    /* wait for token */
    while (!bluetooth.available()) {
        bluetooth.pollACI();
        status = bluetooth.getState();
    }

    /* read token into buffer */
    while (bluetooth.available()) {
        buffer[pos++] = bluetooth.read();
    }
    buffer[pos] = '\0';

    return status;
}
