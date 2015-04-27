#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>

#define LCD_SCE   20
#define LCD_RST   19
#define LCD_DC    18
#define LCD_MOSI  16
#define LCD_SCLK  15

#define LCD_C     LOW
#define LCD_D     HIGH

#define LCD_X     84
#define LCD_Y     48

#define GPSSerial Serial1
#define USBSerial Serial

#define GPSECHO  true

static const byte ASCII[][5] =
{
 {0x00, 0x00, 0x00, 0x00, 0x00} // 20  
,{0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
,{0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
,{0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
,{0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
,{0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
,{0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
,{0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
,{0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
,{0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
,{0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
,{0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
,{0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
,{0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
,{0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
,{0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
,{0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
,{0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
,{0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
,{0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
,{0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
,{0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
,{0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
,{0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
,{0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
,{0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
,{0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
,{0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
,{0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
,{0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
,{0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
,{0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
,{0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
,{0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
,{0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
,{0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
,{0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
,{0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
,{0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
,{0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
,{0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
,{0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
,{0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
,{0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
,{0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
,{0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
,{0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
,{0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
,{0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
,{0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
,{0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
,{0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
,{0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
,{0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
,{0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
,{0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
,{0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
,{0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
,{0x02, 0x04, 0x08, 0x10, 0x20} // 5c ¥
,{0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
,{0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
,{0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
,{0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
,{0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
,{0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
,{0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
,{0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
,{0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
,{0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
,{0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
,{0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
,{0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
,{0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j 
,{0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
,{0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
,{0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
,{0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
,{0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
,{0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
,{0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
,{0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
,{0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
,{0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
,{0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
,{0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
,{0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
,{0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
,{0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
,{0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
,{0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
,{0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
,{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
,{0x10, 0x08, 0x08, 0x10, 0x08} // 7e ←
,{0x78, 0x46, 0x41, 0x46, 0x78} // 7f →
};

Adafruit_GPS GPS(&GPSSerial);

void lcd_write_char(char character)
{
  lcd_write_cmd(LCD_D, 0x00);
  for (int index = 0; index < 5; index++)
  {
    lcd_write_cmd(LCD_D, ASCII[character - 0x20][index]);
  }
  lcd_write_cmd(LCD_D, 0x00);
}

void lcd_clear_display(void)
{
  for (int index = 0; index < LCD_X * LCD_Y / 8; index++)
  {
    lcd_write_cmd(LCD_D, 0x00);
  }
}

void lcd_clear_row(int y)
{
  int i ;
  lcd_pos(0, y) ;
  for (i = 0 ; i < LCD_X ; i++) {
    lcd_write_cmd(LCD_D, 0x00) ;
  }
}

void lcd_pos(int x, int y)
{
  lcd_write_cmd( 0, 0x80 | x);  // Column.
  lcd_write_cmd( 0, 0x40 | y);  // Row.  
}

void lcd_init(void)
{
  pinMode(LCD_SCE, OUTPUT);
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_DC, OUTPUT);
  pinMode(LCD_MOSI, OUTPUT);
  pinMode(LCD_SCLK, OUTPUT);
  digitalWrite(LCD_RST, LOW);
  digitalWrite(LCD_RST, HIGH);
  lcd_write_cmd(LCD_C, 0x21 );  // LCD Extended Commands.
  lcd_write_cmd(LCD_C, 0xB1 );  // Set LCD Vop (Contrast). 
  lcd_write_cmd(LCD_C, 0x04 );  // Set Temp coefficent. //0x04
  lcd_write_cmd(LCD_C, 0x14 );  // LCD bias mode 1:48. //0x13
  lcd_write_cmd(LCD_C, 0x20 );  // LCD Basic Commands
  lcd_write_cmd(LCD_C, 0x0C );  // LCD in normal mode.
}

void lcd_write_str(char *characters)
{
  while (*characters)
  {
    lcd_write_char(*characters++);
  }
}

void lcd_write_cmd(byte dc, byte data)
{
  digitalWrite(LCD_DC, dc);
  digitalWrite(LCD_SCE, LOW);
  shiftOut(LCD_MOSI, LCD_SCLK, MSBFIRST, data);
  digitalWrite(LCD_SCE, HIGH);
}

void setup(void)
{
  lcd_init();
  lcd_clear_display();
  lcd_write_str("Niggers!");
  
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
      print_distance_1km() ;
    }

  }
}

#define LAT_1KM  44.893158
#define LNG_1KM -68.6634645
void print_distance_1km(void)
{
  static const double lat_1km = degrees_to_radians(LAT_1KM) ;
  static const double lng_1km = degrees_to_radians(LNG_1KM) ;

  double gps_lat = degrees_to_radians(GPS.latitudeDegrees) ;
  double gps_lng = degrees_to_radians(GPS.longitudeDegrees) ;

  double distance = calc_dist_coord(gps_lat, gps_lng, 
                                    lat_1km, lng_1km) ;
  lcd_print_float(distance/1e3) ;
}

void lcd_print_float(double d)
{
  static int row = 0 ;
  static char buf[12] ;
  lcd_clear_row(row) ;
  lcd_pos(0, row) ;
  dtostrf(d, 1, 7, buf) ;
  lcd_write_str(buf) ;
  row = row == 4 ? 1 : row + 1 ;
}

#define MEAN_EARTH_RADIUS 6371e3
/* all arguments must be in radians */
double calc_dist_coord(double lat_a, double lng_a, double lat_b, double lng_b)
{
  double del_lat, del_lng, a ;

  /* use haversine formula */
  del_lat = lat_a - lat_b ;
  del_lng = lng_a - lng_b ;

  a = 2*asin(sqrt(square(sin(del_lat/2))
        + cos(lat_a)*cos(lat_b)*square(sin(del_lng/2)))) ;

  return a*MEAN_EARTH_RADIUS ;
}

#if 0
double GPPGA_to_degrees(double gppga)
{
  double integral, fractional ;
  fractional = modf(gppga/100.0, &integral) ;
  return integral + fractional*100.0/60.0 ;
}
#endif

double degrees_to_radians(double degrees)
{
  return degrees*2.0*M_PI/360.0 ;
}