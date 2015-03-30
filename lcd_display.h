#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#define LCD_DISPLAY_WIDTH 84
#define LCD_DISPLAY_HEIGHT 48

/* setup lcd for use */
void lcd_display_setup(void) ;

/* turn off all pixels */
void lcd_display_clear(void) ;

/* map a byte grid to the lcd */
void
lcd_display_map(unsigned char grid[LCD_DISPLAY_HEIGHT][LCD_DISPLAY_WIDTH]) ;

/* write data out to the lcd */
/* sets and clears will not display until updated */
void lcd_display_update(void) ;

#endif
