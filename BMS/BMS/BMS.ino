// BMS.ino 
// Temperature Calculations and ADC for the BMS
// Authors: Alex Sternberg
// 			    Thinh Le
// Property of Spartan Racing Electric 2018
#include <Linduino.h>
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <stdint.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "UserInterface.h"
#include <LTC68041.h>
#include <SPI.h>
#include "src/mcp2515.h"
#include "src/Canbus.h"  
#include "src/LTC68041_COMM.h"
// #include "BMS_TEMP.h"

#define TOTAL_IC 8
#define TOTAL_BMS_CHANNEL 9 // Supposed to be 9
#define TOTAL_LTC_CHANNEL 12
#define DEBUG 1
#define TEMP_RD_BASE_ADDR 0x28
#define TEMP_WR_BASE_ADDR 0x29
#define MAIN_CURR A0
#define CHGR_CURR A1

float voltages[TOTAL_IC][TOTAL_LTC_CHANNEL];
float current;
float resistance[TOTAL_IC][9];
uint8_t tx_cfg[TOTAL_IC][6] = { { 0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00 },
                  { 0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00 } };  // test with print tx
uint8_t rx_cfg[TOTAL_IC][8] = { { 0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00, 0x00, 0x00 },
                  { 0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00, 0x00, 0x00 } };  // test with print rx
// uint8_t i2c_cfg[TOTAL_IC][] = {};

void setup() {
  Serial.begin(115200);
  //LTC6804_initialize();  //Initialize LTC6804 hardware

  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH); //setup fault
  
  spi_enable(SPI_CLOCK_DIV16);
  set_adc(MD_NORMAL, DCP_DISABLED, CELL_CH_ALL, AUX_CH_ALL);
  
  init_cfg();        //initialize the 6804 configuration array to be written

// so you can build a binary without logging statements
// since printing is the slowest operation one can do on a mcu
// hence compiler dierctives to tell it to not build those lines in
  if (mcp2515_init(CANSPEED_500)) {
#if (DEBUG) 
    Serial.println("CAN init OK"); 
#endif
  }
#if (DEBUG)
  else {
    Serial.println("CAN init FAIL");
  }
#endif
}

// Scope for void loop
int8_t read_cells(float cellv[TOTAL_IC][TOTAL_LTC_CHANNEL]) {  
  //allocate temp measure memory 
  uint16_t cell_codes[TOTAL_IC][TOTAL_LTC_CHANNEL]; 
  // Wake isoSPI up from idle state
  wakeup_idle();  
  //command cell measurement
  LTC6804_adcv(); 
  //read measured voltages
  int8_t error = LTC6804_rdcv(0, TOTAL_LTC_CHANNEL, cell_codes);   
  if (error) {
    set_fault();
  }  
  // Counter to see which line code steps into/over
  // int debug_count = 0;
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    for (int i = 0; i < TOTAL_LTC_CHANNEL; i++) { 
      // Serial.print("Iteration count: ");
      // Serial.print(current_ic);
      // Serial.println(i);

      //convert fixed point to floating point
      float voltage = cell_codes[current_ic][i] * 0.0001;
      // Fill in channels 10, 11, 12 with empty
      if(i > TOTAL_BMS_CHANNEL)
      {
        cellv[current_ic][i] = 0;
      }
      cellv[current_ic][i] = voltage; // <- The issue
            
      // Serial.println(cellv[current_ic][i]);

      // For 20V to BMB, 2.35 for 35V
      if (voltage < 2.22 || voltage > 4.35) {
        //voltage out of range
        set_fault(); 
      }
      if (!(i % 5)) i++;
      if (i == 11) i++;
    }
  }  
}

void set_fault() {
  // Set pin 6 (CS) active low, BITCLR
  digitalWrite(6, LOW);
}

float read_current(uint8_t sensor) {
  // Definition in wiring_analog.c 
  int measure = analogRead(sensor);
  return 200 * (measure / 820);
}

#if (DEBUG)

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

#endif

//----------------------------------------------------------

// I2C Command 
union COMM_WR_REG {
    uint8_t bytes[6];
    // Bit fields of Write register
    struct COMM {
        uint8_t ICOM0   : 4;
        uint8_t D0      : 8;
        uint8_t FCOM0   : 4;
        uint8_t ICOM1   : 4;
        uint8_t D1      : 8;
        uint8_t FCOM1   : 4;
        uint8_t ICOM2   : 4;
        uint8_t D2      : 8;
        uint8_t FCOM2   : 4;
        // uint16_t PEC    : 16;
    } fields;
};

union COMM_RD_REG {
    uint8_t bytes[8];
    // Bit fields of Read register 
    struct COMM {
        uint8_t ICOM0   : 4;
        uint8_t D0      : 8;
        uint8_t FCOM0   : 4;
        uint8_t ICOM1   : 4;
        uint8_t D1      : 8;
        uint8_t FCOM1   : 4;
        uint8_t ICOM2   : 4;
        uint8_t D2      : 8;
        uint8_t FCOM2   : 4;
        uint16_t PEC    : 16;
    } fields;
};

#define SDA_GPIO4 0x20
#define SCL_GPIO5 0x21 // 1/(2*tclk), tclk = 1us
// #define I2C_READ_COMMAND {  }
static uint8_t index = 0;
int8_t BMS_I2C_NEXT();

// Our execution of functions
void loop() {
  // char input;
  // do
  // {
  //   input = Serial.read();
  //   read_cells(voltages); // TODO
  //   read_current(current);
  //   // test 
  //   print_txdata(tx_cfg);
  //   print_rxdata(rx_cfg);    
  // } while(input != 'q');

  // i2c test 
  LTC6804_initialize();
  // LTC6804_wrcfg(TOTAL_IC, i2c_cfg);
  int8_t bms_i2c = BMS_I2C_NEXT();
  Serial.print("i2c error: ");
  Serial.println(bms_i2c);
}

// Definition I2C 
int8_t BMS_I2C_NEXT(/*float rx_i2c[TOTAL_IC]*/) // TODO: What is the point of this arg?
{
  // Write code for I2C Master
  Serial.println("sending wrcomm");
	COMM_WR_REG reg;
	reg.fields.ICOM0 = 0x6; 							// send start
	reg.fields.D0 = SDA_GPIO4;
	reg.fields.FCOM0 = 0x8; 							// send nack
	reg.fields.ICOM1 = 0x0;  							// send blank
	reg.fields.D1 =    0x00; 							// send empty byte
	reg.fields.FCOM1 = 0x9;  							// send NACK+STOP
	reg.fields.ICOM2 = 0x7;  							// no send
	reg.fields.D2 =    0x00; 							// empty byte
	reg.fields.FCOM2 = 0x0;  							// send ack

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

  // There's already data in the read configuration array
	if (LTC6804_rdcomm(TOTAL_IC, rd_data)) {
		Serial.println("Error reading I2C device\n");
		res |= 1 << 8; //set pec error
	}
	else {
		Serial.println("Got data!\n");
		Serial.println((const char *)rd_data);
		// print_rxdata(rd_data);
	}

  // Read code for I2C Master
	COMM_RD_REG rd_reg;

	for (int i = 0; i < TOTAL_IC; i++) {
		memcpy(rd_reg.bytes, rd_data[i], 8);//load data from sensor
		uint16_t measure = (rd_reg.fields.D1 << 8) | (rd_reg.fields.D2); //cast data bytes to uint16
		// rx_temp[i] = convert(measure); //convert to *C

		//collect errors
		res |= rd_reg.fields.FCOM0;
		res |= rd_reg.fields.FCOM1;
		res |= rd_reg.fields.FCOM2;
	}
	
	index = ++index % 9;

	return res; //return any errors
}

//----------------------------------------------------------

/*!***********************************
\brief Initializes the configuration array
**************************************/
void init_cfg()
{
  uint8_t tx_cfg[TOTAL_IC][6] = { { 0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00 },
                  { 0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00 } };
  LTC6804_wrcfg(TOTAL_IC, tx_cfg);
}
