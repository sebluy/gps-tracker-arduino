/*!
 * @file
 *
 * @brief Header file for the Bluetooth module
 *
 * @author Andrew Hayford
 * @author Sebastian Luy
 *
 * @date 12 December, 2015
 *
 * This file contains the function prototypes needed to
 * use the Bluetooth module
 * 
 */

#include <lib_aci.h>

/*!
 * @brief enum holding acceptable statuses of Bluetooth module
 *
 * Possible states of Bluetooth are listed in the enum below.
 *
 */
enum bluetooth_status_t {SLEEPING, SETUP, STANDBY, ADVERTISING, CONNECTED};

/*!
 * @brief struct containing data needed for Bluetooth module
 *
 * Contains the status of the Bluetooth module, bookkeeping values,
 * and the message flag/buffer
 *
 */
struct bluetooth_t {
    aci_state_t aci_state;      /* low level state */
    char uart_buffer[21];       /* buffer for receiving messages */
    bluetooth_status_t status;  
    bool has_message;           /* tell if message ready */
    bool setup_required;        /* used internally for setup procedure */
    bool timing_change_done;    /* used internally for making timing changes */
};

/*!
 * @brief Performs setup and puts bluetooth to sleep
 *
 * Sets up the Bluetooth module by configuring hardware information
 * and setting values in the bluetooth struct. Puts the device to sleep
 * after initialization.
 *
 * @param[in,out]  bluetooth  Pointer to bluetooth struct to initialize
 *
 * @returns    Nothing.
 *
 */
void bluetooth_setup(bluetooth_t *bluetooth);

/*!
 * @brief Wakes up bluetooth and begins advertising
 *
 * Wakes up the Bluetooth module and sets the device to advertise
 * at an interval of 160ms
 *
 * @param[in,out]  bluetooth  Pointer to bluetooth struct we want to advertise
 *
 * @returns    Nothing.
 *
 */
void bluetooth_advertise(bluetooth_t *bluetooth);

/*!
 * @brief Disconnects bluetooth (if necessary) and puts it to sleep
 *
 * If connected, disconnects the bluetooth and puts the module to sleep
 *
 * @param[in,out]  bluetooth  Pointer to bluetooth struct to sleep
 *
 * @returns    Nothing.
 *
 */
void bluetooth_sleep(bluetooth_t *bluetooth);


/*!
 * @brief Returns last received status
 *
 * Simply returns the status of the Bluetooth module
 *
 * @param[in]  bluetooth  Pointer to bluetooth struct
 *
 * @returns    Status of the Bluetooth module
 *
 */
bluetooth_status_t bluetooth_get_status(bluetooth_t *bluetooth);

/*!
 * @brief Checks if Bluetooth has message
 *
 * Returns true iff bluetooth has recieved a message 
 * and has not read it yet
 *
 * @param[in]  bluetooth  Pointer to bluetooth struct
 *
 * @returns    True if unread message, false otherwise
 */
bool bluetooth_has_message(bluetooth_t *bluetooth);

/*!
 * @brief Fetch a message over bluetooth
 *
 * Returns last message recieved from bluetooth.Only a 
 * pointer is returned, so care must be taken to ensure
 * the value is consumed before the next message is read.
 * If bluetooth module receives a new message,and 
 * bluetooth_poll is called, the new message will stomp 
 * the current message. 
 *
 * @param[in]  bluetooth  Pointer to bluetooth struct
 *
 * @returns    Pointer to a message
 */
char *bluetooth_get_message(bluetooth_t *bluetooth);

/*!
 * @brief Updates Bluetooth statuses 
 *
 * Updates bluetooth status and message after
 * bluetooth module changes state. This call must be
 * made explicitly for any updates to occur.
 *
 * @param[in,out]  bluetooth  Pointer to bluetooth struct
 *
 * @returns    Nothing
 */   
void bluetooth_poll(bluetooth_t *bluetooth);
