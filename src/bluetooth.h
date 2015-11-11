#include <lib_aci.h>

enum bluetooth_status_t {SLEEPING, SETUP, STANDBY, ADVERTISING, CONNECTED};

struct bluetooth_t {
    aci_state_t aci_state;
    char uart_buffer[21];
    bluetooth_status_t status;
    bool has_message;
    bool setup_required;
    bool timing_change_done;
};

/* performs setup and puts bluetooth to sleep */
void bluetooth_setup(bluetooth_t *bluetooth);

/* wakes up bluetooth and begins advertising */
void bluetooth_advertise(bluetooth_t *bluetooth);

/* Disconnects bluetooth (if necessary) and puts it to sleep.
 * Bluetooth must be in STANDBY (or will be soon) or CONNECTED state. */
void bluetooth_sleep(bluetooth_t *bluetooth);

bluetooth_status_t bluetooth_get_status(bluetooth_t *bluetooth);

bool bluetooth_has_message(bluetooth_t *bluetooth);
char *bluetooth_get_message(bluetooth_t *bluetooth);

//void bluetooth_initialize(bluetooth_t *bluetooth);
void bluetooth_poll(bluetooth_t *bluetooth);
