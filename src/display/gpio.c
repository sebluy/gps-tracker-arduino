#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "file.h"

#define GPIO_DIR "/sys/class/gpio/"
#define GPIO_EXPORT "/sys/class/gpio/export"

static void gpio_file(char *file, char *pin, char *append) ;
static void gpio_export(char *pin) ;
static void gpio_output(char *pin) ;
static int gpio_exported(char *pin) ;

void gpio_setup(char *pin)
{
	if (!gpio_exported(pin))
		gpio_export(pin) ;
	gpio_output(pin) ;
}

void gpio_set_value(char *pin, char *value)
{
	int fd ;
	char file[80] ;
		
	gpio_file(file, pin, "/value") ;
	
	fd = Open(file, O_WRONLY) ;
	Write(fd, value, 1) ;
	Close(fd) ;
}

static void gpio_file(char *file, char *pin, char *append)
{	
	sprintf(file, "%sgpio%s%s", GPIO_DIR, pin, append) ;
}

static void gpio_export(char *pin)
{
	int fd ;

	fd = Open(GPIO_EXPORT, O_WRONLY) ;
	Write(fd, pin, strlen(pin)) ;
	Close(fd) ;
}

static void gpio_output(char *pin)
{
	int fd ;
	char file[80] ;
		
	gpio_file(file, pin, "/direction") ;
	
	fd = Open(file, O_WRONLY) ;
	Write(fd, "out", 3) ;
	Close(fd) ;
}

static int gpio_exported(char *pin)
{
	char dir[80] ;
	sprintf(dir, "%sgpio%s/", GPIO_DIR, pin) ;
	return !access(dir, F_OK) ;
}
