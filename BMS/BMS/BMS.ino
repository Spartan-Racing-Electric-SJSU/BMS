#include <Arduino.h>
#include <stdint.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "UserInterface.h"
#include "LTC68041.h"
#include "LTC68041_COMM.h"
#include <SPI.h>

#define TOTAL_IC 2

void setup() {
	Serial.begin(115200);
	LTC6804_initialize();  //Initialize LTC6804 hardware
	init_cfg();        //initialize the 6804 configuration array to be written
}

void loop() {
	Serial.println("sending wrcomm");
	uint8_t data[TOTAL_IC][6] = { {0x6A, 0x08, 0x00, 0x18, 0x0A, 0xA9},
								  {0x6A, 0x08, 0x00, 0x18, 0x0A, 0xA9} };
	
	print_txdata(data);

	LTC6804_wrcomm(TOTAL_IC, data);
	LTC6804_stcomm(TOTAL_IC);
	
	uint8_t rd_data[TOTAL_IC][8];

	if (LTC6804_rdcomm(TOTAL_IC, rd_data)) {
		Serial.println("Error reading I2C device");
	}
	else {
		Serial.println("Got data!");
		print_rxdata(rd_data);
	}

}

void print_txdata(uint8_t rx_cfg[][6])
{
	Serial.println("Sent Bytes ");
	for (int current_ic = 0; current_ic<TOTAL_IC; current_ic++)
	{
		Serial.print(" IC ");
		Serial.print(current_ic + 1, DEC);
		Serial.print(": 0x");
		serial_print_hex(rx_cfg[current_ic][0]);
		Serial.print(", 0x");
		serial_print_hex(rx_cfg[current_ic][1]);
		Serial.print(", 0x");
		serial_print_hex(rx_cfg[current_ic][2]);
		Serial.print(", 0x");
		serial_print_hex(rx_cfg[current_ic][3]);
		Serial.print(", 0x");
		serial_print_hex(rx_cfg[current_ic][4]);
		Serial.print(", 0x");
		serial_print_hex(rx_cfg[current_ic][5]);
		Serial.println();
	}
	Serial.println();
}

void print_rxdata(uint8_t rx_cfg[][8])
{
	Serial.println("Received Bytes ");
	for (int current_ic = 0; current_ic<TOTAL_IC; current_ic++)
	{
		Serial.print(" IC ");
		Serial.print(current_ic + 1, DEC);
		Serial.print(": 0x");
		serial_print_hex(rx_cfg[current_ic][0]);
		Serial.print(", 0x");
		serial_print_hex(rx_cfg[current_ic][1]);
		Serial.print(", 0x");
		serial_print_hex(rx_cfg[current_ic][2]);
		Serial.print(", 0x");
		serial_print_hex(rx_cfg[current_ic][3]);
		Serial.print(", 0x");
		serial_print_hex(rx_cfg[current_ic][4]);
		Serial.print(", 0x");
		serial_print_hex(rx_cfg[current_ic][5]);
		Serial.print(", Received PEC: 0x");
		serial_print_hex(rx_cfg[current_ic][6]);
		Serial.print(", 0x");
		serial_print_hex(rx_cfg[current_ic][7]);
		Serial.println();
	}
	Serial.println();
}

void serial_print_hex(uint8_t data)
{
	if (data< 16)
	{
		Serial.print("0");
		Serial.print((byte)data, HEX);
	}
	else
		Serial.print((byte)data, HEX);
}


/*!***********************************
\brief Initializes the configuration array
**************************************/
void init_cfg()
{
	uint8_t tx_cfg[TOTAL_IC][6] = { { 0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00 } };
	LTC6804_wrcfg(TOTAL_IC, tx_cfg);
}