// BMS_TEMP.cpp
// Temperature Calculations and ADC for the BMS
// Authors: Alex Sternberg
// 			Thinh Le
// Property of Spartan Racing Electric 2018
#include <math.h>
#include "BMS_TEMP.h"
#include "src/LTC68041_COMM.h"
#include "HardwareSerial.h"

static uint8_t index = 0;

int8_t BMS_TEMP_next(float rx_temp[TOTAL_IC]) {

	Serial.println("sending wrcomm");
	COMM_WR_REG reg;
	reg.fields.ICOM0 = 0x6; 							// send start
	reg.fields.D0 = TEMP_RD_BASE_ADDR + (index << 1); 	// to adc channel addr
	reg.fields.FCOM0 = 0x8; 							// send nack
	reg.fields.ICOM1 = 0x0;  							// send blank
	reg.fields.D1 =    0x00; 							// send empty byte
	reg.fields.FCOM1 = 0x8;  							// send NACK+STOP
	reg.fields.ICOM2 = 0x0;  							// no send
	reg.fields.D2 =    0x00; 							// empty byte
	reg.fields.FCOM2 = 0x9;  							// send ack

	uint8_t tx_data[TOTAL_IC][6];
	//now build copies for each ic
	for (int i = 1; i < TOTAL_IC; i++) {
		memcpy(tx_data[i], reg.bytes, 6);
		Serial.write((const char *)tx_data[i]);
		Serial.println("Wrote data!\n");
	}	

	// print_txdata(tx_data);

	//transmit to pack
	LTC6804_wrcomm(TOTAL_IC, tx_data);
	LTC6804_stcomm(TOTAL_IC);

	uint8_t rd_data[TOTAL_IC][8];

	int8_t res;

	if (LTC6804_rdcomm(TOTAL_IC, rd_data)) {
		Serial.println("Error reading I2C device\n");
		res |= 1 << 8; //set pec error
	}
	else {
		Serial.println("Got data!\n");
		Serial.println((const char *)rd_data);
		// print_rxdata(rd_data);
	}

	COMM_RD_REG rd_reg;

	for (int i = 0; i < TOTAL_IC; i++) {
		memcpy(rd_reg.bytes, rd_data[i], 8);//load data from sensor
		uint16_t measure = (rd_reg.fields.D1 << 8) | (rd_reg.fields.D2); //cast data bytes to uint16
		rx_temp[i] = convert(measure); //convert to *C

		//collect errors
		res |= rd_reg.fields.FCOM0;
		res |= rd_reg.fields.FCOM1;
		res |= rd_reg.fields.FCOM2;
	}
	
	index = ++index % 9;

	return res; //return any errors
}



float convert(uint16_t measure) {
	float resistance = SERIES_RESISTOR / (65535 / measure - 1);
	float steinhart;
	steinhart = resistance / THERMISTOR_NOMINAL;     	// (R/Ro)
	steinhart = log(steinhart);                  		// ln(R/Ro)
	steinhart /= BETA_COEFFICIENT;                   		// 1/B * ln(R/Ro)
	steinhart += 1.0 / (TEMP_NOMINAL + 273.15); 	// + (1/To)
	steinhart = 1.0 / steinhart;                 		// Invert
	steinhart -= 273.15;                         		// convert to C
	return steinhart;
}