#include <SPI.h>
#include <avr/eeprom.h>
#include "Adafruit_BLE_UART.h"

#define REQ 9
#define RDY 7
#define RST 10

void setup(void)
{ 
}

void loop()
{
    Serial.begin(9600);
    while(!Serial);

    Adafruit_BLE_UART bluetooth = Adafruit_BLE_UART(REQ, RDY, RST);
    bluetooth.setDeviceName("PETRICE"); /* 7 characters max! */
    bluetooth.begin();

    while (1) {
        get_and_store_waypoints(bluetooth);
    }
}

void get_and_store_waypoints(Adafruit_BLE_UART bluetooth)
{
    char buffer[21];
    uint32_t count, received;
    float latitude, longitude;
    uint32_t *eeprom_address = 0;

    Serial.println("Ready for action");

    /* get count */
    bluetooth_get_token(bluetooth, buffer);
    count = atoi(buffer);
    sprintf(buffer, "%d", count);
    Serial.println(buffer);

    Serial.println("Writting to EEPROM");
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

    Serial.println("Reading out latlngs from eeprom");

    eeprom_address = (uint32_t*)4;
    /* get latitudes and longitudes */
    for (int read = 0; read < count; read++) {
        latitude = eeprom_read_float((float*)eeprom_address);
        Serial.println(latitude);
        eeprom_address += 1;

        longitude = eeprom_read_float((float*)eeprom_address);
        Serial.println(longitude);
        eeprom_address += 1;
    }

    Serial.println("All good homie");
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
