#ifndef LTC68041_COMM_H
#define LTC68041_COMM_H

#include <stdint.h>



union COMM_REG{
  uint8_t bytes[6];
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

void LTC6804_wrcomm(uint8_t,uint8_t[][6]);
void LTC6804_stcomm(uint8_t);
int8_t LTC6804_rdcomm(uint8_t,uint8_t[][8]);


#endif
