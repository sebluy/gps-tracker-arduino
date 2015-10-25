#include <Adafruit_FRAM_I2C.h>
#include <Wire.h>
#include "Arduino.h"
#include "adafruit_fram_logger.h"

Adafruit_FRAM_I2C fram     = Adafruit_FRAM_I2C(); 

uint32_t get_unix_time(uint32_t hh, uint32_t mm, uint32_t ss, 
                       uint32_t dd, uint32_t mo, uint32_t yy) 
{
  uint32_t unix_time = 0 ;
  uint32_t mo_arr [12] = {0,2678400,5097600,7776000,10368000,13046400,
                         15638400,18316800,20995200,23587200,26265600,
                         28857600} ;
  uint32_t day_sum = 0 ;
  uint32_t leap_days = 0 ;
  uint32_t cur_leap = 0 ;

  unix_time = (2000-1970+yy)*31536000 ;
  unix_time += mo_arr[mo-1] ;

   /* Determine leap days */
  leap_days = (2000+yy-1972)/4 + 1 ;
  cur_leap = (2000+yy)/4 ;

  /* Subtract a leap day if it hasn't occurred in current year yet */
  if (cur_leap == leap_days) {
    if (mo <= 2)
      leap_days-- ;
  }

  day_sum = dd+leap_days-1 ;
  unix_time += (day_sum*86400) ;
  
  unix_time += (mm*60) ;
  unix_time += ss ;
  unix_time += 3600*hh ;
  
  return unix_time ;  
}

void fram_init(void)
{
   fram.begin() ;
}

void fram_write_float(uint16_t addr, float value)
{
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       fram.write8(addr++, *p++);
}

float fram_read_float(uint16_t addr)
{
   float value = 0.0;
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       *p++ = fram.read8(addr++);
   return value;
}
  
void fram_write_uint32(uint16_t addr, uint32_t value)
{
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       fram.write8(addr++, *p++);
}

uint32_t fram_read_uint32(uint16_t addr)
{
   uint32_t value = 0.0;
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       *p++ = fram.read8(addr++);
   return value;
}

void fram_log (float lat, float lon, uint32_t unix_time) 
{
  Serial.print(unix_time) ;
  Serial.print("\n") ;
  Serial.print(lat) ;
  Serial.print("\n") ; 
  Serial.print(lon) ;
  Serial.print("\n") ;
  
  fram_write_float(0, lat) ;
  fram_write_float(4, lon) ;
  fram_write_uint32(8, unix_time) ;

  Serial.print(fram_read_uint32(8)) ;
  Serial.print("\n") ;
  Serial.print(fram_read_float(0)) ;
  Serial.print("\n") ;
  Serial.print(fram_read_float(4)) ;
  Serial.print("\n\n") ;
}
