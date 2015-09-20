#include <SPI.h>
#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>
#include "lcd.h"
extern "C" {
#include "haversine.h"
}

#define GPSSerial Serial1
#define USBSerial Serial

#define GPSECHO  true
#define MEAN_EARTH_RADIUS 6371e3
#define M_TO_FT 3.28084

Adafruit_GPS GPS(&GPSSerial);
HardwareSerial mySerial = Serial1;

volatile int tracking_en = 0 ; 
volatile int bluetooth_en = 0 ;

void setup(void)
{
  attachInterrupt(1, isr_stop, HIGH) ;
  attachInterrupt(0,isr_start,HIGH) ;
  attachInterrupt(4,isr_bluetooth,HIGH);

  lcd_init();
  lcd_clear_display();
  lcd_write_str("Cache Rules Everything Around Me");

  USBSerial.begin(115200);
  delay(1000);

  GPS.begin(9600);

  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_100_MILLIHERTZ) ;
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);
  GPSSerial.println(PMTK_Q_RELEASE);
  lcd_clear_display() ; 
}

void loop(void)
{
  lcd_clear_display() ;
  lcd_write_str("Dist:") ;
  delay(2500) ;
  
  lcd_pos(1,1) ;
  lcd_print_float(6.03) ;
  delay(2500) ;
  
  lcd_pos(1,2) ;
  lcd_write_str("Speed:") ;
  delay(2500) ;
  
  lcd_pos(1,3) ;
  lcd_print_float(1.001) ;
  delay(2500) ;
  
  
  if (tracking_en) {
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
  
  //Serial.print(tracking_en) ;
  //Serial.write(":") ;
  //Serial.print(bluetooth_en) ;
  //Serial.write("\n") ;
  //delay(1500) ;
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
  delay(200) ;
  
  lcd_pos(0,1) ;
  lcd_print_float(dist_coord) ;
  delay(200) ;
  
  lcd_pos(0,3) ;
  lcd_write_str("Speed:") ;
  delay(200) ;
  
  lcd_pos(0,4) ;
  lcd_print_float(speed) ;
  delay(200) ;
  
  //lcd_print_float(GPS.latitude) ;
  //lcd_print_float(GPS.longitude) ;
}

void isr_start(void)
{
  if (debounce(3) ) {
    detachInterrupt(0) ;
    noInterrupts() ;
    if ( tracking_en == 0 ) {
      tracking_en = 1 ;
      bluetooth_en = 0 ;
      lcd_clear_display() ;
      lcd_pos(0,0) ;
      lcd_write_str("Tracking mode enabled") ;
    }
    attachInterrupt(0, isr_start, HIGH) ;
    interrupts() ;  
  }
}

void isr_bluetooth(void)
{
  if ( debounce(7) ) {
    detachInterrupt(4) ;
    noInterrupts() ;
    tracking_en = 0 ;
    bluetooth_en = 1;
    lcd_clear_display() ;
    lcd_pos(0,0) ;
    lcd_write_str("Bluetooth") ;
    attachInterrupt(4, isr_bluetooth, HIGH) ;
    interrupts() ;
  }
}

void isr_stop(void)
{
  if ( debounce(2) ) {
    detachInterrupt(1) ;
    noInterrupts() ;
    
    if ( tracking_en == 1) {
      tracking_en = 0 ;
      lcd_clear_display() ;
      lcd_pos(0,0) ;
      lcd_write_str("Tracking mode disabled") ;
    } else if ( bluetooth_en == 1) {
      bluetooth_en = 0 ;
      lcd_clear_display() ;
      lcd_pos(0,0) ;
      lcd_write_str("Bluetooth stopped by user") ;
    }
    
    attachInterrupt(1, isr_stop, HIGH) ;
    interrupts() ;
  }
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


