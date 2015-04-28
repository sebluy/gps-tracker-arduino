#include "lcd.h"

void setup(void)
{
  lcd_init();
}

void loop(void)
{
  lcd_print_double(1.2345, 0) ;
  lcd_print_double(9.345, 1) ;
  lcd_print_double(20.20, 4) ;
  lcd_print_double(58.34, 5) ;
}
