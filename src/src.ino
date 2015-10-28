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

#define GPSECHO  false
#define MEAN_EARTH_RADIUS 6371e3   /* Mean radius of Earth (haversine calculation) */
#define M_TO_FT 3.28084            /* Conversion ratio from metres to feet */

/* Define Adafruit GPS object */
Adafruit_GPS GPS(&GPSSerial);

uint8_t tracking_en = 0 ;
uint8_t new_data = 0 ;

void setup(void)
{
  /* Initialise LCD - Print startup message */
  lcd_init();
  lcd_clear_display();
  lcd_write_str("Cache Rules Everything Around Me");
  delay(2500) ;
  lcd_clear_display();
  
  /* Wait for searial to be ready - start GPS/USB at 9600 baud */
  USBSerial.begin(9600);
  GPS.begin(9600);
  
  /* Configure GPS - 1Hz update frequency, full NMEA output */
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ) ;
  GPS.sendCommand(PGCMD_ANTENNA);
  GPSSerial.println(PMTK_Q_RELEASE);
  
  /* Initialise the FRAM module */
  //fram_init() ;
  
  attachInterrupt(1, isr_tracking, HIGH);
  attachInterrupt(0, isr_pps, RISING) ;
  GPS.sendCommand(PMTK_STANDBY) ;
}

void loop(void)
{ 
  if(tracking_en) {
    char c = GPS.read();
    if ((c) && (GPSECHO)) {
      USBSerial.write(c);
    }
    
    if (GPS.newNMEAreceived()) {
      
      // add back in && GPS.fix
      if (GPS.parse(GPS.lastNMEA()) && GPS.fix && new_data) {
        print_GPS_results() ;
        new_data = 0 ;
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
 
  double avg_speed = 0.0 ;
 
  static uint8_t log_count = 0 ;
  static int first = 1 ;
  
  static uint32_t unix_time = 0 ;
  static uint32_t time_elapsed = 0 ;

  static uint32_t start_time = 0 ;

  double speed = GPS.speed / 1.15076 ;
  double coord_diff = 0.0 ;
  if (first) {
    first = !first ;
    start_time = get_unix_time(GPS.hour, GPS.minute, GPS.seconds,
                GPS.day, GPS.month, GPS.year) ; 
  } else {
    dist_speed += (speed / 3600.0) * 10.0 * 5280 ;
    coord_diff = calc_dist_coord(GPS.latitude, GPS.longitude,
                                 last_lat, last_lng) * M_TO_FT ;
    dist_coord +=  coord_diff ;
    speed = (coord_diff * 360.0 ) / 5280.0 ;
  
    unix_time = get_unix_time(GPS.hour, GPS.minute, GPS.seconds,
                GPS.day, GPS.month, GPS.year) ;
  }
  
  time_elapsed = unix_time - start_time ;
  
  last_lat = GPS.latitude ;
  last_lng = GPS.longitude ;
  
  lcd_pos(0,1) ;
  lcd_print_float(time_elapsed) ;
  
  lcd_pos(0,3) ;
  lcd_print_float(dist_coord) ;
  
  lcd_pos(0,4) ;
  lcd_print_float(speed) ;
  
  lcd_print_time(GPS.hour, GPS.minute, GPS.seconds) ;
  
  /*
  log_count++
  if(log_count == 10) {
    fram_log(last_lat, last_lng, unix_time) ;
    log_count = 0 ;
  }
  */
}

bool debounce(int pin) 
{
  bool flag = 1 ;
  for (int i = 0; i < 1000; i++) {
     if (digitalRead(pin) != HIGH ) {
       flag = 0 ;
       return flag ;
     }
  }
  return flag ;
}

void isr_tracking(void)
 {
  if ( debounce(2) ) {
    noInterrupts() ;
    if ( tracking_en == 1) {
      tracking_en = 0 ;
      GPS.sendCommand(PMTK_STANDBY) ;
    } else {
      tracking_en = 1 ;
      GPS.sendCommand(PMTK_AWAKE) ;
    }    
    interrupts() ;
  }
 }
 
 void isr_pps(void)
 {
   new_data = 1 ;
 }
