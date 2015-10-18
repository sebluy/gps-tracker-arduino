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
#define GPSECHO  false
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
}

void loop(void)
{ 
 if (ZERO) {
   char c = GPS.read();

   if ((c) && (GPSECHO)) {
     USBSerial.write(c);
   }

   if (GPS.newNMEAreceived()) {
     // add back in && GPS.fix
     if (GPS.parse(GPS.lastNMEA()) ) {
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
  
  get_unix_time(GPS.hour, GPS.minute, GPS.seconds,
                GPS.day, GPS.month, GPS.year) ;
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

uint32_t get_unix_time(uint32_t hh, uint32_t mm, uint32_t ss, 
                       uint32_t dd, uint32_t mo, uint32_t yy) 
{
  uint32_t unix_time = 0 ;
  uint32_t mo_arr [12] = {0,2678400,5097600,7776000,10368000,13046400,
                         15638400,18316800,20995200,23587200,26265600,
                         28857600} ;
  uint32_t day_sum = 0 ;
  uint32_t leap_days = 0 ;
  uint32_t cur_leap = 0 ;

  unix_time = (2000-1970+yy)*31536000 ;
  unix_time += mo_arr[mo-1] ;

   /* Determine leap days */
  leap_days = (2000+yy-1972)/4 + 1 ;
  cur_leap = (2000+yy)/4 ;

  /* Subtract a leap day if it hasn't occurred in current year yet */
  if (cur_leap == leap_days) {
    if (mo <= 2)
      leap_days-- ;
  }

  day_sum = dd+leap_days-1 ;
  unix_time += (day_sum*86400) ;
  
  unix_time += (mm*60) ;
  unix_time += ss ;
  unix_time += 3600*hh ;
  
  return unix_time ;  
}

void fram_write_str(uint16_t addr, char *str) 
{
  uint16_t i = addr ; 
  while(*str) {
    fram.write8(i,*str++) ;
    i++ ;
  }
}

void fram_read_len(uint16_t base_addr, uint16_t len)
{
  uint16_t value = 0 ;
  uint16_t i = base_addr ;
  while(i < (base_addr + len)) {
    value = fram.read8(i++) ;
    Serial.print(value, HEX) ;
    Serial.print(" ") ;
  }
}


    
