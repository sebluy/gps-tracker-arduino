#ifndef SPI_H
#define SPI_H

/* setup spi for use with default values and a max speed of 2 MHz */
void spi_setup(void) ;

/* write count bytes of data in buf to spi device */
void spi_write(void *buf, int count) ;

#endif
