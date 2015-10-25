#include <SPI.h>
#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>
#include <Adafruit_FRAM_I2C.h>
#include <Wire.h>

#include "nokia_5110.h"
#include "adafruit_fram_logger.h" 

/* include C file */
extern "C" {
#include "haversine.h"
}

#define GPSSerial Serial1          /* Serial talking to GPS */
#define USBSerial Serial           /* USB serial port */

#define GPSECHO  false             /* Disable echo of GPS data to serial */
#define MEAN_EARTH_RADIUS 6371e3   /* Mean radius of Earth (haversine calculation) */
#define M_TO_FT 3.28084            /* Conversion ratio from metres to feet */

/* Define Adafruit GPS and I2C FRAM objects */
Adafruit_GPS GPS(&GPSSerial);


void setup(void)
{
  /* Initialise LCD - Print startup message */
  lcd_init();
  lcd_clear_display();
  lcd_write_str("Cache Rules Everything Around Me");
  delay(500) ;
  lcd_clear_display();
  
  /* Wait for searial to be ready - start GPS/USB at 9600 baud */
  while(!USBSerial) ;
  USBSerial.begin(9600);
  GPS.begin(9600);
  
  /* Configure GPS - 100MHz update frequency, full NMEA output */
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_100_MILLIHERTZ) ;
  GPS.sendCommand(PGCMD_ANTENNA);
  GPSSerial.println(PMTK_Q_RELEASE);
  
  /* Initialise the FRAM module */
  fram_init() ;
}

void loop(void)
{ 
  char c = GPS.read();

  if ((c) && (GPSECHO)) {
     USBSerial.write(c);
  }

  if (GPS.newNMEAreceived()) {
     // add back in && GPS.fix
     if (GPS.parse(GPS.lastNMEA())) {
       print_GPS_results() ;
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
  uint32_t unix_time = 0;

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
  
  unix_time = get_unix_time(GPS.hour, GPS.minute, GPS.seconds,
                GPS.day, GPS.month, GPS.year) ;
  
  fram_log(last_lat, last_lng, unix_time) ;
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


