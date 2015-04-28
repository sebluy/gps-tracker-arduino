#ifndef LCD_H
#define LCD_H

/* put these tags down because arduino is fucked */
#ifdef __cplusplus
extern "C" {
#endif

void lcd_init(void) ;
void lcd_print_double(double d, int row) ;

#ifdef __cplusplus
}
#endif

#endif
