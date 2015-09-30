#include <SPI.h>
#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>
#include "lcd.h"
#include <Wire.h>
#include <Adafruit_FRAM_I2C.h>
extern "C" {
#include "haversine.h"
}

#define GPSSerial Serial1
#define USBSerial Serial

#define ZERO  1
#define GPSECHO  true
#define MEAN_EARTH_RADIUS 6371e3
#define M_TO_FT 3.28084

Adafruit_GPS GPS(&GPSSerial);

Adafruit_FRAM_I2C fram     = Adafruit_FRAM_I2C();
uint16_t          framAddr = 0;

void setup(void)
{
  lcd_init();
  lcd_clear_display();
  lcd_write_str("Cache Rules Everything Around Me");
  while(!USBSerial) ;
  USBSerial.begin(9600);
  
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_100_MILLIHERTZ) ;
  GPS.sendCommand(PGCMD_ANTENNA);
  GPSSerial.println(PMTK_Q_RELEASE);
  
  lcd_clear_display() ; 
  
  fram.begin() ;
  fram_write_str(0,"hello world") ;
  fram_read_len(0, 11) ;
}

void loop(void)
{ 
 if (ZERO) {
   char c = GPS.read();

   if ((c) && (GPSECHO)) {
     USBSerial.write(c);
   }

   if (GPS.newNMEAreceived()) {

     if (GPS.parse(GPS.lastNMEA()) && GPS.fix) {
       print_GPS_results() ;
     }
   }
  }
}

void print_GPS_results(void)
{
  static double dist_speed = 0.0 ;
  static double dist_coord = 0.0 ;
  static double last_lat ;
  static double last_lng ;
  static int first = 1 ;

  double speed = GPS.speed / 1.15076 ;
  double coord_diff = 0.0 ;
  if (first) {
    first = !first ;
  } else {
    dist_speed += (speed / 3600.0) * 10.0 * 5280 ;
    coord_diff = calc_dist_coord(GPS.latitude, GPS.longitude,
                                 last_lat, last_lng) * M_TO_FT ;
    dist_coord +=  coord_diff ;
    speed = (coord_diff * 360.0 ) / 5280.0 ;
  }

  last_lat = GPS.latitude ;
  last_lng = GPS.longitude ;

  lcd_clear_display() ;
  lcd_write_str("Dist:") ;
  
  lcd_pos(0,1) ;
  lcd_print_float(dist_coord) ;
  
  lcd_pos(0,3) ;
  lcd_write_str("Speed:") ;
  
  lcd_pos(0,4) ;
  lcd_print_float(speed) ;
}

bool debounce(int pin) 
{
  bool flag = 1 ;
  for (int i = 0; i < 100; i++) {
     if (digitalRead(pin) != HIGH ) {
       flag = 0 ;
       return flag ;
     }
  }
  return flag ;
}

void fram_write_str(uint16_t addr, char *str) 
{
  uint8_t i = addr ; 
  while(*str) {
    fram.write8(i,*str++) ;
    i++ ;
  }
}

void fram_read_len(uint16_t base_addr, uint16_t len)
{
  uint8_t value = 0 ;
  int i = base_addr ;
  while(i < (base_addr + len)) {
    value = fram.read8(i++) ;
    Serial.print(value, HEX) ;
    Serial.print(" ") ;
  }
}
    
