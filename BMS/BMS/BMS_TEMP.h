// BMS_TEMP.h
// Temperature Calculations and ADC for the BMS
// Authors: Alex Sternberg
// 			Thinh Le
// Property of Spartan Racing Electric 2018
#ifndef _BMS_TEMP_h
#define _BMS_TEMP_h

#define TEMP_BASE_ADDR 0x28
#define TEMP_RD_BASE_ADDR TEMP_BASE_ADDR | 0x1
#define TEMP_WR_BASE_ADDR TEMP_BASE_ADDR
#define TEMP_BASE_CONF { 0x60, 0x08, 0x00, 0x08, 0x00, 0x09 }
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000   
#define TOTAL_IC 8

#include <stdint.h>

union COMM_WR_REG {
	uint8_t bytes[6];
	struct COMM {
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

union COMM_RD_REG {
	uint8_t bytes[8];
	struct COMM {
		uint8_t ICOM0 : 4;
		uint8_t D0 : 8;
		uint8_t FCOM0 : 4;
		uint8_t ICOM1 : 4;
		uint8_t D1 : 8;
		uint8_t FCOM1 : 4;
		uint8_t ICOM2 : 4;
		uint8_t D2 : 8;
		uint8_t FCOM2 : 4;
		uint16_t PEC : 16;
	} fields;
};

#define TEMP_READ_COMMAND { 0x62, 0x88, 0x00, 0x18, 0x0A, 0xA9 }

int8_t BMS_TEMP_next(float rx_temp[TOTAL_IC]);
float convert(uint16_t measure);

#endif

