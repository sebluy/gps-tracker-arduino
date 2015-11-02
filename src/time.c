#include <stdint.h>

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
