#include <SPI.h>
#include <avr/eeprom.h>

#include "lcd.h"
#include "types.h"
#include "waypoint_reader.h"

#define GREEN_BUTTON_INTERRUPT_NUM 1  /* Corresponds to pin 2 (D2) */
#define BLUE_BUTTON_INTERRUPT_NUM 0   /* Corresponds to pin 3 (D3) */

/*
 * Global flags indicating a button press has not yet been handled.
 * Will be set to 1 on button press, and should be read and cleared
 * atomically (disable/renable interrupts).
 */
uint8_t g_green_button_pressed = 0;

void print_waypoint(point_t waypoint, int n)
{
    /* waypoint number */
    lcd_pos(0, 0);
    lcd_print_float((float)n, 0);

    /* latitude */
    lcd_pos(0, 1);
    lcd_print_float(waypoint.latitude, 6);

    /* longitude */
    lcd_pos(0, 2);
    lcd_print_float(waypoint.longitude, 6);
}

void setup(void)
{
    SPI.begin();

    /* Initialise LCD - Print startup message */
    lcd_init();
    lcd_clear_display();

    attachInterrupt(GREEN_BUTTON_INTERRUPT_NUM, green_button_handler, FALLING);

    waypoint_reader_t waypoint_reader;
    waypoint_reader_initialize(&waypoint_reader);
    point_t current_waypoint = waypoint_reader_get_next(&waypoint_reader);
    int n = 1;

    print_waypoint(current_waypoint, n);

    while (1) {

        noInterrupts();

        /* green button press in this context means enter tracking mode */
        if (g_green_button_pressed) {
            g_green_button_pressed = 0;
            interrupts();

            /* restart if at end */
            if (waypoint_reader_end(&waypoint_reader)) {
                waypoint_reader_initialize(&waypoint_reader);
                n = 0;
            }

            /* increment waypoint */
            current_waypoint = waypoint_reader_get_next(&waypoint_reader);
            n++;

            print_waypoint(current_waypoint, n);
        }

        interrupts();

    }
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
    if (current - last > 200) {
        g_green_button_pressed = 1;
    }
    last = current;
}
