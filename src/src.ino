#include <SPI.h>
#include "Adafruit_BLE_UART.h"

#define REQ 9
#define RDY 7
#define RST 10

Adafruit_BLE_UART bluetooth = Adafruit_BLE_UART(REQ, RDY, RST);

void setup(void)
{
  Serial.begin(9600);
  while (!Serial);
  bluetooth.setDeviceName("Ralph");
}

void loop(void)
{
  if (Serial.available()) {
    String s = Serial.readString();
    if (s == "Send") {
      send();
    }
  }
}

void send(void)
{
  Serial.println("Waiting");
  bluetooth.begin();

  aci_evt_opcode_t status = ACI_EVT_DISCONNECTED;

  /* wait for connect */
  while (status != ACI_EVT_CONNECTED) {
    bluetooth.pollACI();
    status = bluetooth.getState();
  }
  Serial.println("Connected");

  /* dump payload */
  bluetooth.println("start");
  send_point("43.7", "-70.6", "1.5");
  send_point("43.6", "-70.6", "1.4");
  send_point("43.6", "-70.5", "1.3");
  send_point("43.7", "-70.5", "1.2");
  send_point("43.7", "-70.6", "1.1");
  bluetooth.println("finish");
  Serial.println("Done");
}

/* for some reason without print_status between each call,
 * bluetooth.print bugs out. Maybe the call to pollACI fixes */
void send_point(String latitude, String longitude, String speed)
{
  bluetooth.print(latitude);
  print_status();
  bluetooth.print(longitude);
  print_status();
  bluetooth.print(speed);
  print_status();
  delay(1000);
}

void print_status(void)
{
  bluetooth.pollACI();
  aci_evt_opcode_t status = bluetooth.getState();
  Serial.println(status);
}


