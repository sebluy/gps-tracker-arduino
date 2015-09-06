#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>
extern "C" {
  #include "lcd.h"
}

#define GPSSerial Serial1
#define USBSerial Serial

#define GPSECHO  true
#define MEAN_EARTH_RADIUS 6371e3
#define M_TO_FT 3.28084

Adafruit_GPS GPS(&GPSSerial);
HardwareSerial mySerial = Serial1;

void setup(void)
{
  lcd_init();
  lcd_clear_display();
  lcd_write_str("init");
  delay(1000);
  
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

double calc_dist_coord(double lat_a, double lng_a, double lat_b, double lng_b)
{
  double a, c, del_lat, del_lng ;
  double lat_a_radians = GPPGA_to_radians(lat_a) ;
  double lng_a_radians = GPPGA_to_radians(lng_a) ;
  double lat_b_radians = GPPGA_to_radians(lat_b) ;
  double lng_b_radians = GPPGA_to_radians(lng_b) ;

  /* use haversine formula */
  del_lat = fabs(lat_a_radians - lat_b_radians) ;
  del_lng = fabs(lng_a_radians - lng_b_radians) ;

  a = 2*asin(sqrt(square(sin(del_lat/2))
        + cos(lat_a)*cos(lat_b)*square(sin(del_lng/2)))) ;

  /* alternative form */
  /*
  a = square(sin(del_lat/2)) + cos(lat_a)*cos(lat_b)*square(sin(del_lng/2)) ;
  c = 2.0*atan2(sqrt(a), sqrt(1.0 - a)) ;
  */

  return a*MEAN_EARTH_RADIUS ;
}

double GPPGA_to_radians(double degrees)
{
  double integral, fractional ;
  fractional = modf(degrees/100.0, &integral) ;
  return (integral + fractional*100.0/60.0)*2*M_PI/360.0 ;
}
