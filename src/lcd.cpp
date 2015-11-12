/*!
 * @file
 *
 * @brief Interface for the Nokia 5110 Graphic LCD
 *
 * @author Andrew Hayford
 * @author Sebastian Luy
 *
 * @date 11 November, 2015
 *
 * This file contains the function definitions required to interface
 * with the LCD. This includes initialization, printing (characters, floats,
 * times, strings), and sending commands. 
 * 
 */

#include "SPI.h"
#include "Arduino.h"
#include "lcd.h"
#include "avr/pgmspace.h"
#include "string.h"

/*!
 * @brief Writes a single character to the LCD
 *
 * This function takes in a single character and
 * writes it to the current position on the LCD.
 *
 * @param[in]  character  A character to write to the LCD
 *
 * @returns    Nothing.
 *
 */
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

/*!
 * @brief Clears the entire display
 *
 * This function clears the LCD, row by row.
 *
 * @returns    Nothing.
 *
 */
void lcd_clear_display(void)
{
  int i ;
  for (i = 0; i < LCD_Y; i++)
  {
    lcd_clear_row(i) ;
  }
  lcd_pos(0,0) ;
}

/*!
 * @brief Clears a single row on the display
 *
 * Clears the selected row y on the LCD. y must
 * be an integer with value between 0 and 4.
 *
 * @param[in]  y  The row to clear on the LCD (must be a value between 0 and 4)
 *
 * @returns    Nothing.
 *
 */
void lcd_clear_row(int y)
{
  int i ;
  lcd_pos(0, y) ;
  for (i = 0 ; i < LCD_X ; i++) {
    lcd_write_cmd(HIGH, 0x00) ;
  }
}

/*!
 * @brief Moves the cursor on the display to position (x,y)
 *
 * This function sets the cursor position (x,y) corresponding
 * to the x and y values in the arguments.
 *
 * @param[in]  x  The column for the cursor to appear on (0-12)
 * @param[in]  y  The row for the cursor to appear on (0-4)
 *
 * @returns    Nothing.
 *
 */
void lcd_pos(int x, int y)
{
  lcd_write_cmd(LOW, 0x80 | x);  // Column.
  lcd_write_cmd(LOW, 0x40 | y);  // Row.
}

/*!
 * @brief Initialises the LCD
 *
 * Configures the LCD by setting SCE, RST, DC, MOSI, SCLK as outputs,
 * then sending the commands to setup the LCD for display of data
 *
 * @returns    Nothing.
 */
void lcd_init(void)
{
  //SPI.begin();
  //SPI.setBitOrder(MSBFIRST) ;

  pinMode(LCD_SCE, OUTPUT);
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_DC, OUTPUT);
//  pinMode(LCD_MOSI, OUTPUT);
//  pinMode(LCD_SCLK, OUTPUT);
  digitalWrite(LCD_RST, LOW);
  digitalWrite(LCD_RST, HIGH);
  lcd_write_cmd(LOW, 0x21 );  // LCD Extended Commands.
  lcd_write_cmd(LOW, 0xB1 );  // Set LCD Vop (Contrast).
  lcd_write_cmd(LOW, 0x04 );  // Set Temp coefficent. //0x04
  lcd_write_cmd(LOW, 0x14 );  // LCD bias mode 1:48. //0x13
  lcd_write_cmd(LOW, 0x20 );  // LCD Basic Commands
  lcd_write_cmd(LOW, 0x0C );  // LCD in normal mode.
}

/*!
 * @brief Writes a string to the LCD
 *
 * Writes a generic string to the LCD
 *
 * @param[in]  characters  Pointer to a character array (string)
 *
 * @returns    Nothing.
 *
 */
void lcd_print_str(char *str)
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

/*!
 * @brief Writes a command to the LCD
 *
 * Commands are issued on the Nokia 5110 by writing the
 * relevant mode select (D/CBAR), pulling the chip-enable
 * (SCE) low (active low), entering the command byte
 * (D7-D0), then bringing chip enable high again
 *
 * @param[in]  dc    The value to set dc
 * @param[in]  data  A command byte in the form DB7, DB6 .. ,DB0
 *
 * @returns    Nothing.
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

/*!
 * @brief Prints a floating point number to the LCD
 *
 * Writes a floating point number d to the LCD
 *
 * @param[in]  d      Floating point value to display
 * @param[in]  numdec Number of digits after the decimal point 
 *
 * @returns    Nothing.
 *
 */
void lcd_print_float(double d, int numdec)
{
  static char buf[LCD_LEN-3] ; /* buffer to hold converted float */
  
  /* Convert passed double to string with numdec precision */
  dtostrf(d, 1, numdec, buf) ;
  
  /* Fill remaining of buffer with spaces to kill unwanted data */
  for(int i = strlen(buf); i < LCD_LEN-3; i++) {
    buf[i] = 0x20 ;
  }
  
  lcd_print_str(buf) ;
}

/*!
 * @brief Writes the time in hh:mm:ss to the LCD
 *
 * Takes in three integers indicating the hours, minutes, and
 * seconds of a time string, and displays them to the LCD.
 *
 * @param[in]  hh    Integer expressing the hours of a time string
 * @param[in]  mm    Integer expressing the minutes of a time string
 * @param[in]  ss    Integer expressing the seconds of a time string
 *
 * @returns    Nothing.
 *
 */
void lcd_print_time(int hh, int mm, int ss)
{
  char buf[LCD_LEN-3] ;  /* buffer to hold resulting string - 10 available characters per line */
  
  /* Write integer values to string in time format */
  sprintf(buf, "%02d:%02d:%02d", hh, mm, ss) ;
  lcd_print_str(buf) ;
}
