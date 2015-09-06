#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>
extern "C" {
  #include "lcd.h"
  #include "haversine.h"
}

#define GPSSerial Serial1
#define USBSerial Serial

#define BUT_STOP  3
#define BUT_STAR  2
#define BUT_BLUE  7

#define GPSECHO  true
#define MEAN_EARTH_RADIUS 6371e3
#define M_TO_FT 3.28084

Adafruit_GPS GPS(&GPSSerial);
HardwareSerial mySerial = Serial1;

void setup(void)
{
  pinMode(BUT_STOP, INPUT) ;
  pinMode(BUT_STAR, INPUT) ;
  pinMode(BUT_BLUE, INPUT) ;

  attachInterrupt(1,isr_stop,HIGH) ;
  attachInterrupt(0,isr_start,HIGH) ;
  attachInterrupt(4,isr_bluetooth,HIGH);
  
  lcd_init();
  lcd_clear_display();
  lcd_write_str("init");
   
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
  /*
  char c = GPS.read();

  if ((c) && (GPSECHO)) {
    USBSerial.write(c); 
  }
  
  if (GPS.newNMEAreceived()) {

    if (GPS.parse(GPS.lastNMEA()) && GPS.fix) {
       print_GPS_results() ;
    }

  }
  */
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
    dist_speed += (speed/3600.0)*10.0*5280 ; 
    coord_diff = calc_dist_coord(GPS.latitude, GPS.longitude,
                                    last_lat, last_lng)*M_TO_FT ;
    dist_coord +=  coord_diff ;
    speed = (coord_diff * 360.0 )/5280.0 ;
  }
  
  last_lat = GPS.latitude ;
  last_lng = GPS.longitude ;
    
  lcd_print_float(dist_coord) ;
  lcd_print_float(speed) ;
  lcd_print_float(GPS.latitude) ;
  lcd_print_float(GPS.longitude) ;
}

void isr_start(void)
{
  lcd_pos(0,0) ;
  lcd_write_str("start") ;
  delay(3000);              // wait for a second
  lcd_clear_display() ;
}

void isr_stop(void)
{
  lcd_pos(0,0) ;
  lcd_write_str("stop") ;
  delay(3000);              // wait for a second
  lcd_clear_display() ;
}

void isr_bluetooth(void)
{
  lcd_pos(0,0) ;
  lcd_write_str("bluetooth") ;
  delay(3000);
  lcd_clear_display() ;
}

      
      
    

