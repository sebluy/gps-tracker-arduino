#include <lib_aci.h>

enum bluetooth_status_t {SLEEPING, SETUP, STANDBY, ADVERTISING, CONNECTED};

struct bluetooth_t {
    aci_state_t aci_state; /* low level state */
    char uart_buffer[21]; /* buffer for receiving messages */
    bluetooth_status_t status;
    bool has_message;
    bool setup_required; /* used internally for setup procedure */
    bool timing_change_done; /* used internally for making timing changes */
};

/* Performs setup and puts bluetooth to sleep. */
void bluetooth_setup(bluetooth_t *bluetooth);

/* Wakes up bluetooth and begins advertising. */
void bluetooth_advertise(bluetooth_t *bluetooth);

/* Disconnects bluetooth (if necessary) and puts it to sleep. */
void bluetooth_sleep(bluetooth_t *bluetooth);

/* Returns last recieved status */
bluetooth_status_t bluetooth_get_status(bluetooth_t *bluetooth);

/* Returns true iff bluetooth has recieved a message and has not read it yet */
bool bluetooth_has_message(bluetooth_t *bluetooth);

/* Returns last message recieved from bluetooth.
   Only a pointer is returned, so care must be taken to ensure
   the value is consumed before the next message is read.
   If bluetooth module receives a new message,
   and bluetooth_poll is called, the new message will
   stomp the current message. */
char *bluetooth_get_message(bluetooth_t *bluetooth);

/* Updates bluetooth status and message after
   bluetooth module changes state. This call must be
   made explicitly for any updates to occur. */
void bluetooth_poll(bluetooth_t *bluetooth);
