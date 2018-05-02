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
#include "LTC68041.h"
#include "LTC68041_COMM.h"
#include <SPI.h>
#include "mcp2515.h"
#include "Canbus.h"  

#define TOTAL_IC 8
#define DEBUG 1
#define TEMP_RD_BASE_ADDR 0x28
#define TEMP_WR_BASE_ADDR 0x29
#define MAIN_CURR A0
#define CHGR_CURR A1

float voltages[TOTAL_IC][9];
float current;
float resistance[TOTAL_IC][9];
uint8_t tx_cfg[TOTAL_IC][6] = { { 0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00 },
                  { 0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00 } };  // test with print tx
uint8_t rx_cfg[TOTAL_IC][8] = { { 0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00, 0x00, 0x00 },
                  { 0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00, 0x00, 0x00 } };  // test with print rx

void setup() {
  Serial.begin(115200);
  //LTC6804_initialize();  //Initialize LTC6804 hardware

  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH); //setup fault
  
  spi_enable(SPI_CLOCK_DIV16);
  set_adc(MD_NORMAL, DCP_DISABLED, CELL_CH_ALL, AUX_CH_ALL);
  
  init_cfg();        //initialize the 6804 configuration array to be written

  if (mcp2515_init(CANSPEED_500)) {
#if (DEBUG) 
    Serial.println("CAN init OK"); 
#endif
  }
#if (DEBUG)
  else {
    // Serial.println("CAN init FAIL");
    print_txdata(tx_cfg);
  print_rxdata(rx_cfg);
  }
#endif
}

void loop() {
  read_cells(voltages);
  read_current(current);
  // test 
  print_txdata(tx_cfg);
  print_rxdata(rx_cfg);
}

int8_t read_cells(float cellv[TOTAL_IC][9]) {
  uint16_t cell_codes[TOTAL_IC][12]; //allocate temp measure memory 
  wakeup_idle();
  LTC6804_adcv(); //command cell measurement
  int8_t error = LTC6804_rdcv(0, TOTAL_IC, cell_codes); //read measured voltages
  if (error) {
    set_fault();
  }
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    for (int i = 0; i < 12; i++) { //convert fixed point to floating point
      float voltage = cell_codes[current_ic][i] * 0.0001;
      cellv[current_ic][i] = voltage;
      // For 20V to BMB
      if (voltage < 2.22 || voltage > 4.35) {
        set_fault(); //voltage out of range
      }
      if (!(i % 5)) i++;
      if (i == 11) i++;
    }
  }
}

void set_fault() {
  digitalWrite(6, LOW);
}

float read_current(uint8_t sensor) {
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


/*!***********************************
\brief Initializes the configuration array
**************************************/
void init_cfg()
{
  uint8_t tx_cfg[TOTAL_IC][6] = { { 0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00 },
                  { 0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00 } };
  LTC6804_wrcfg(TOTAL_IC, tx_cfg);
}
