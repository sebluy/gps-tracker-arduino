#ifndef ADAFRUIT_FRAM_LOGGER_H
#define ADAFRUIT_FRAM_LOGGER_H

#ifdef _cpluscplus
extern "C" {
#endif

uint32_t get_unix_time(uint32_t hh, uint32_t mm, uint32_t ss, 
                       uint32_t dd, uint32_t mo, uint32_t yy) ;

void fram_init(void) ;

void fram_write_float(uint16_t addr, float value) ;

float fram_read_float(uint16_t addr) ;
  
void fram_write_uint32(uint16_t addr, uint32_t value) ;

uint32_t fram_read_uint32(uint16_t addr) ;

void fram_log (float lat, float lon, uint32_t unix_time) ;

#ifdef _cplusplus
}
#endif

#endif
