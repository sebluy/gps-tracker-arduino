#include <SPI.h>
#include "Adafruit_BLE_UART.h"

#define REQ 9
#define RDY 7
#define RST 10

Adafruit_BLE_UART bluetooth = Adafruit_BLE_UART(REQ, RDY, RST);

void setup(void)
{
  bluetooth.setDeviceName("Ralph");
}

void loop(void)
{
  send();
}

void send(void)
{
  bluetooth.begin();

  aci_evt_opcode_t status = ACI_EVT_DISCONNECTED;

  /* wait for connect */
  while (status != ACI_EVT_CONNECTED) {
    bluetooth.pollACI();
    status = bluetooth.getState();
  }

  /* dump payload */
  bluetooth.print("start");
  send_point("43.7", "-70.6", "1.5");
  send_point("43.6", "-70.6", "1.4");
  send_point("43.6", "-70.5", "1.3");
  send_point("43.7", "-70.5", "1.2");
  send_point("43.7", "-70.6", "1.1");
  bluetooth.print("finish");
  Serial.println("Done");
}

/* for some reason without pollACI between each call,
 * bluetooth.print bugs out after ~25 chars. */
void send_point(String latitude, String longitude, String speed)
{
  bluetooth.print(latitude);
  bluetooth.pollACI();
  bluetooth.print(longitude);
  bluetooth.pollACI();
  bluetooth.print(speed);
  bluetooth.pollACI();
}


