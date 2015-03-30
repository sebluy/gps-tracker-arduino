#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#include "file.h"
#include "spi.h"

#define MAX_FREQUENCY 2000000 /* 2 Mhz */

static int fd ;

static void set_max_frequency(int frequency) ;

void spi_setup(void)
{
	fd = Open("/dev/spidev0.0", O_RDWR) ;
	set_max_frequency(MAX_FREQUENCY) ;
}

void spi_write(void *buf, int count)
{
	Write(fd, buf, count) ;
}

static void set_max_frequency(int frequency)
{
	Ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &frequency) ;
}
