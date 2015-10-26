#include "SPI.h"
#include "Arduino.h"
#include "nokia_5110.h"
#include "avr/pgmspace.h"
#include "string.h"

void lcd_write_char(char character)
{
  int i ;
  lcd_write_cmd(HIGH, 0x00);
  for (i = 0; i < 5; i++)
  {
    lcd_write_cmd(HIGH, pgm_read_byte(&(ASCII[character - 0x20][i])));
  }
  lcd_write_cmd(HIGH, 0x00);
}

void lcd_clear_display(void)
{
  int i ;
  for (i = 0; i < LCD_Y; i++) 
  {
    lcd_clear_row(i) ;
  }
  lcd_pos(0,0) ;
}

void lcd_clear_row(int y)
{
  int i ;
  lcd_pos(0, y) ;
  for (i = 0 ; i < LCD_X ; i++) {
    lcd_write_cmd(HIGH, 0x00) ;
  }
}

void lcd_pos(int x, int y)
{
  lcd_write_cmd(LOW, 0x80 | x);  // Column.
  lcd_write_cmd(LOW, 0x40 | y);  // Row.  
}

void lcd_init(void)
{
  //SPI.begin();
  //SPI.setBitOrder(MSBFIRST) ;
  
  pinMode(LCD_SCE, OUTPUT);
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_DC, OUTPUT);
  pinMode(LCD_MOSI, OUTPUT);
  pinMode(LCD_SCLK, OUTPUT);
  digitalWrite(LCD_RST, LOW);
  digitalWrite(LCD_RST, HIGH);
  lcd_write_cmd(LOW, 0x21 );  // LCD Extended Commands.
  lcd_write_cmd(LOW, 0xB1 );  // Set LCD Vop (Contrast). 
  lcd_write_cmd(LOW, 0x04 );  // Set Temp coefficent. //0x04
  lcd_write_cmd(LOW, 0x14 );  // LCD bias mode 1:48. //0x13
  lcd_write_cmd(LOW, 0x20 );  // LCD Basic Commands
  lcd_write_cmd(LOW, 0x0C );  // LCD in normal mode.
}

/* 
 * Writes string to the LCD 
 *
 * Writes a string to the LCD by individually
 * issuing each character in the string
 *
 */
void lcd_write_str(char *str)
{
  unsigned int len = 0 ;
  unsigned int cur_line_len = 0 ;
  unsigned int line = 0 ;
  char * pch = NULL ;
  
  /* Find length of string */
  len = strlen(str) ;
  
  /* Print normally if it can fit on one line */
  while (*str) {
    lcd_write_char(*str++);
  }
}

/* 
 * Issues a command to the LCD
 *
 * Commands are issued on the Nokia 5110 by writing the 
 * relevant mode select (D/CBAR), pulling the chip-enable
 * (SCE) low (active low), entering the command byte 
 * (D7-D0), then bringing chip enable high again
 * 
 */
void lcd_write_cmd(byte dc, byte data)
{
   digitalWrite(LCD_DC, dc);    /* Mode select */
  
  /* Write command/address/data byte */
  //shiftOut(LCD_MOSI, LCD_SCLK, MSBFIRST, data);
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  digitalWrite(LCD_SCE, LOW);  /* Chip enable active low */
  SPI.transfer(data) ;
  digitalWrite(LCD_SCE, HIGH);  /* Chip enable high */
  SPI.endTransaction();
}

/* 
 * Prints a floating point number to the LCD
 *
 */
void lcd_print_float(double d)
{
  static char buf[13] ;
  dtostrf(d, 1, 3, buf) ;
  lcd_write_str(buf) ;
}

void lcd_print_time(int hh, int mm, int ss)
{
  char buf[13] ;
 
  /* Convert to EST */
  if (hh < 4) {
    hh = 24 - hh ;
  } else {
    hh = hh - 4 ;
  }
  
  lcd_pos(2,0) ;
  sprintf(buf, "%02d:%02d:%02d", hh, mm, ss) ;
  lcd_write_str(buf) ;
}




