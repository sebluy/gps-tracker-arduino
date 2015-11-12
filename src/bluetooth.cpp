#include <SPI.h>
#include <lib_aci.h>
#include <aci_setup.h>
#include <services.h>

#include "bluetooth.h"

/* Code based on both
   https://github.com/NordicSemiconductor/ble-sdk-arduino/tree/master/libraries/BLE/examples/ble_uart_project_template
   and
   https://github.com/adafruit/Adafruit_nRF8001/blob/master/Adafruit_BLE_UART.cpp */

/* static setup inormation */
#ifdef SERVICES_PIPE_TYPE_MAPPING_CONTENT
static services_pipe_type_mapping_t services_pipe_type_mapping[NUMBER_OF_PIPES] = SERVICES_PIPE_TYPE_MAPPING_CONTENT;
#else
#define NUMBER_OF_PIPES 0
static services_pipe_type_mapping_t * services_pipe_type_mapping = NULL;
#endif

/* Store the setup for the nRF8001 in the flash of the AVR to save on RAM */
static const hal_aci_data_t setup_msgs[NB_SETUP_MESSAGES] PROGMEM = SETUP_MESSAGES_CONTENT;

/* Define how assert should function in the BLE library */
void __ble_assert(const char *file, uint15_t line)
{
    Serial.print("ERROR ");
    Serial.print(file);
    Serial.print(": ");
    Serial.print(line);
    Serial.print("\n");
    while(1);
}

bluetooth_status_t bluetooth_get_status(bluetooth_t *bluetooth)
{
    return bluetooth->status;
}

void bluetooth_setup(bluetooth_t *bluetooth)
{

    bluetooth->setup_required = false;
    bluetooth->timing_change_done = false;
    bluetooth->status = SETUP;
    bluetooth->has_message = false;

    aci_state_t *aci_state = &bluetooth->aci_state;

    /* fill aci_state with setup information */
    if (NULL != services_pipe_type_mapping)
    {
        aci_state->aci_setup_info.services_pipe_type_mapping = &services_pipe_type_mapping[0];
    }
    else
    {
        aci_state->aci_setup_info.services_pipe_type_mapping = NULL;
    }

    aci_state->aci_setup_info.number_of_pipes = NUMBER_OF_PIPES;
    aci_state->aci_setup_info.setup_msgs = (hal_aci_data_t*)setup_msgs;
    aci_state->aci_setup_info.num_setup_msgs = NB_SETUP_MESSAGES;

    /* fill aci_state with MCU hardware information (pins, clocks, etc...) */
    aci_state->aci_pins.board_name = BOARD_DEFAULT;
    aci_state->aci_pins.reqn_pin = 9;
    aci_state->aci_pins.rdyn_pin = 7;
    aci_state->aci_pins.mosi_pin = MOSI;
    aci_state->aci_pins.miso_pin = MISO;
    aci_state->aci_pins.sck_pin = SCK;

    //SPI_CLOCK_DIV8  = 2MHz SPI speed
    //SPI_CLOCK_DIV16 = 1MHz SPI speed
    aci_state->aci_pins.spi_clock_divider = SPI_CLOCK_DIV8;

    aci_state->aci_pins.reset_pin              = 10;
    aci_state->aci_pins.active_pin             = UNUSED;
    /* chip sel doesn't appear to be exposed on our breakout board */
    aci_state->aci_pins.optional_chip_sel_pin  = UNUSED;

    aci_state->aci_pins.interface_is_interrupt = true;
    aci_state->aci_pins.interrupt_number       = 4;

    /* send initialization command to bluetooth module */
    lib_aci_init(aci_state, false);

    while (1) {
        bluetooth_poll(bluetooth);
        if (STANDBY == bluetooth_get_status(bluetooth)) {
            break;
        }
    }

    bluetooth_sleep(bluetooth);
}


void bluetooth_advertise(bluetooth_t *bluetooth)
{
    lib_aci_wakeup();

    Serial.println("Waiting for standby");

    while (STANDBY != bluetooth->status) {
        bluetooth_poll(bluetooth);
    }

    /* no timeout, 0x100 * 0.626 = 160 ms advertising interval */
    lib_aci_connect(0, 0x100);

    while (ADVERTISING != bluetooth->status) {
        bluetooth_poll(bluetooth);
    }

    Serial.println("Advertising");
}

/* Disconnects bluetooth (if necessary) and puts it to sleep. */
void bluetooth_sleep(bluetooth_t *bluetooth)
{
    /* disconnect if connected */
    bluetooth_status_t status = bluetooth_get_status(bluetooth);
    if (CONNECTED == status) {
        if (lib_aci_disconnect(&bluetooth->aci_state, ACI_REASON_TERMINATE)) {
            Serial.println("Disconnecting to go to sleep");
            while (1) {
                bluetooth_poll(bluetooth);
                if (STANDBY == bluetooth_get_status(bluetooth)) {
                    break;
                }
            }
        } else {
            Serial.println("Could not disconnect");
        }
    }

    if (true == lib_aci_radio_reset()) {
        Serial.println("Radio reset");
    } else {
        Serial.println("Radio reset failed");
    }

    /* sleepy time */
    if (true == lib_aci_sleep()) {
        Serial.println("Sleepy time");
    } else {
        Serial.println("Sleep failed");
    }

    /* for some reason, device wont sleep without poll */
//    bluetooth_poll(bluetooth);

    bluetooth->status = SLEEPING;
}


bool bluetooth_has_message(bluetooth_t *bluetooth)
{
    return bluetooth->has_message == true;
}

char *bluetooth_get_message(bluetooth_t *bluetooth)
{
    bluetooth->has_message = false;
    return bluetooth->uart_buffer;
}

void bluetooth_poll(bluetooth_t *bluetooth)
{
    hal_aci_evt_t aci_data;

    // We enter the if statement only when there is a ACI event available to be processed
    if (lib_aci_event_get(&bluetooth->aci_state, &aci_data))
    {
        aci_evt_t *aci_evt;
        aci_evt = &aci_data.evt;

        Serial.print("Got something: ");
        Serial.println(aci_evt->evt_opcode);

        switch(aci_evt->evt_opcode)
        {
        /* As soon as you reset the nRF8001 you will get an ACI Device Started Event */
        case ACI_EVT_DEVICE_STARTED:
        {
            bluetooth->aci_state.data_credit_total = aci_evt->params.device_started.credit_available;
            Serial.print("Device Mode: ");
            Serial.println(aci_evt->params.device_started.device_mode);
            switch(aci_evt->params.device_started.device_mode)
            {
            case ACI_DEVICE_SETUP:
                /* When the device is in the setup mode */
                Serial.println(F("Evt Device Started: Setup"));
                bluetooth->setup_required = true;
                break;

            case ACI_DEVICE_STANDBY:
                bluetooth->status = STANDBY;
                Serial.println(F("Evt Device Started: Standby"));
                if (aci_evt->params.device_started.hw_error) {
                    Serial.println("Magic number not working");
                    delay(20); //Magic number used to make sure the HW error event is handled correctly.
                }
                break;
            }
        }
        break; //ACI Device Started Event

        case ACI_EVT_CMD_RSP:
            //If an ACI command response event comes with an error -> stop
            if (ACI_STATUS_SUCCESS != aci_evt->params.cmd_rsp.cmd_status)
            {
                //ACI ReadDynamicData and ACI WriteDynamicData will have status codes of
                //TRANSACTION_CONTINUE and TRANSACTION_COMPLETE
                //all other ACI commands will have status code of ACI_STATUS_SUCCESS
                //for a successful command
                Serial.print(F("Evt Cmd response: Status "));
                Serial.println(aci_evt->params.cmd_rsp.cmd_status, HEX);
            }
            if (ACI_CMD_CONNECT == aci_evt->params.cmd_rsp.cmd_opcode) {
                Serial.println(F("Evt Cmd response: Connect"));
                bluetooth->status = ADVERTISING;
            }
            if (ACI_CMD_WAKEUP == aci_evt->params.cmd_rsp.cmd_opcode) {
                Serial.println(F("Evt Cmd response: Awake"));
                bluetooth->status = STANDBY;
            }
            if (ACI_CMD_GET_DEVICE_VERSION == aci_evt->params.cmd_rsp.cmd_opcode)
            {
                Serial.println(F("Evt Cmd response: Get Device Version"));
                //Store the version and configuration information of the nRF8001 in the Hardware Revision String Characteristic
                lib_aci_set_local_data(&bluetooth->aci_state, PIPE_DEVICE_INFORMATION_HARDWARE_REVISION_STRING_SET,
                                       (uint8_t *)&(aci_evt->params.cmd_rsp.params.get_device_version),
                                       sizeof(aci_evt_cmd_rsp_params_get_device_version_t));
            }
            break;

        case ACI_EVT_CONNECTED:
            Serial.println(F("Evt Connected"));
            bluetooth->status = CONNECTED;
            bluetooth->timing_change_done = false;
            bluetooth->aci_state.data_credit_available = bluetooth->aci_state.data_credit_total;

            /* Get the device version of the nRF8001 and store it in the Hardware Revision String */
            lib_aci_device_version();
            break;

        case ACI_EVT_PIPE_STATUS:
            Serial.println(F("Evt Pipe Status"));
            if (lib_aci_is_pipe_available(&bluetooth->aci_state, PIPE_UART_OVER_BTLE_UART_TX_TX) && (false == bluetooth->timing_change_done))
            {
                // change the timing on the link as specified in the nRFgo studio -> nRF8001 conf. -> GAP.
                lib_aci_change_timing_GAP_PPCP();
                // Used to increase or decrease bandwidth
                bluetooth->timing_change_done = true;
            }
            break;

        case ACI_EVT_TIMING:
            Serial.println(F("Evt link connection interval changed"));
            lib_aci_set_local_data(&bluetooth->aci_state,
                                   PIPE_UART_OVER_BTLE_UART_LINK_TIMING_CURRENT_SET,
                                   (uint8_t *)&(aci_evt->params.timing.conn_rf_interval), /* Byte aligned */
                                   PIPE_UART_OVER_BTLE_UART_LINK_TIMING_CURRENT_SET_MAX_SIZE);
            break;

        case ACI_EVT_DISCONNECTED:
            Serial.println(F("Evt Disconnected/Advertising timed out"));
            bluetooth->status = STANDBY;
            break;

        case ACI_EVT_DATA_RECEIVED:
            Serial.print(F("Pipe Number: "));
            Serial.println(aci_evt->params.data_received.rx_data.pipe_number, DEC);
            if (PIPE_UART_OVER_BTLE_UART_RX_RX == aci_evt->params.data_received.rx_data.pipe_number)
            {
                Serial.print(F(" Data(Hex) : "));
                for(int i=0; i<aci_evt->len - 2; i++)
                {
                    Serial.print((char)aci_evt->params.data_received.rx_data.aci_data[i]);
                    bluetooth->uart_buffer[i] = aci_evt->params.data_received.rx_data.aci_data[i];
                    Serial.print(F(" "));
                }
                bluetooth->uart_buffer[aci_evt->len - 1] = '\0';
                bluetooth->has_message = true;

                Serial.println(F(""));
            }
            break;

        case ACI_EVT_DATA_CREDIT:
            bluetooth->aci_state.data_credit_available += aci_evt->params.data_credit.credit;
            break;

        case ACI_EVT_PIPE_ERROR:
            //See the appendix in the nRF8001 Product Specication for details on the error codes
            Serial.print(F("ACI Evt Pipe Error: Pipe #:"));
            Serial.print(aci_evt->params.pipe_error.pipe_number, DEC);
            Serial.print(F("  Pipe Error Code: 0x"));
            Serial.println(aci_evt->params.pipe_error.error_code, HEX);

            //Increment the credit available as the data packet was not sent.
            //The pipe error also represents the Attribute protocol Error Response sent from the peer and that should not be counted
            //for the credit.
            if (ACI_STATUS_ERROR_PEER_ATT_ERROR != aci_evt->params.pipe_error.error_code)
            {
                bluetooth->aci_state.data_credit_available++;
            }
            break;

        case ACI_EVT_HW_ERROR:
            Serial.print(F("HW error: "));
            Serial.println(aci_evt->params.hw_error.line_num, DEC);

            for(uint8_t counter = 0; counter <= (aci_evt->len - 3); counter++)
            {
                Serial.write(aci_evt->params.hw_error.file_name[counter]);
            }
            break;

        }
    }

    /* setup_required is set to true when the device starts up and enters setup mode.
     * It indicates that do_aci_setup() should be called. The flag should be cleared if
     * do_aci_setup() returns ACI_STATUS_TRANSACTION_COMPLETE.
     */
    if (bluetooth->setup_required)
    {
        if (SETUP_SUCCESS == do_aci_setup(&bluetooth->aci_state))
        {
            bluetooth->setup_required = false;
        }
    }
}
