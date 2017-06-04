#ifndef LTC6804_COMM_H
#define LTC6804_COMM_H

#include <stdint.h>



union COMM_REG{
  uint8_t bytes[8];
  struct COMM{
    uint8_t ICOM0 : 4;
    uint8_t D0 : 8;
    uint8_t FCOM0 : 4;
    uint8_t ICOM1 : 4;
    uint8_t D1 : 8;
    uint8_t FCOM1 : 4;
    uint8_t ICOM2 : 4;
    uint8_t D2 : 8;
    uint8_t FCOM2 : 4;
  } fields;
};

void LTC6804_wrcomm(uint8_t,uint8_t*[]);
void LTC6804_stcomm(uint8_t,uint8_t*[]);
void LTC6804_rdcomm(uint8_t,uint8_t*[]);


#endif
