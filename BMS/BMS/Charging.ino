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
#include "BMS_TEMP.h"

#define TEMP_BASE_ADDR 0x28
// Set the read as a set temp base address
#define TEMP_RD_BASE_ADDR TEMP_BASE_ADDR | 0x1
// Set the write as the temp base address
#define TEMP_WR_BASE_ADDR TEMP_BASE_ADDR
// Temp base configuration
#define TEMP_BASE_CFG { 0x60, 0x08, 0x00, 0x08, 0x00, 0x09 }
// Nominal thermistor 
#define THERMISTOR_NOMINAL 10000
// Temp for nominal resistor (25 C)
#define TEMP_NOMINAL 25
// Beta coefficient of thermistor (3000 - 4000)
#define BETA_COEFFICIENT 3950
// 10kOhm resistor
#define SERIES_RESISTOR 10000
// Amount of integrated circuits
#define TOTAL_IC 8
// Temp read command sent via LTC6804 command registry
#define TEMP_READ_COMMAND { 0x62, 0x88, 0x00, 0x18, 0x0A, 0xA9 }
// Configuration Register 4 for DCC bits
#define CFRG4 0x04
// Configuration Register 5 for DCC bits
#define CFRG5 0x05


void setup() {  
  Serial.begin(115200);  
  //setup fault
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);   
}

void set_fault() {
  // Set pin 6 (CS) active low, BITCLR
  digitalWrite(6, LOW); // Sent to BMS Master Fault pin 
}

void serial_print_hex(uint8_t data)
{
  if (data < 16)
  {
    Serial.print("0");
    Serial.print((byte)data, HEX);
  }
  else
    Serial.print((byte)data, HEX);
}

void loop() {
  /* Temp sensing */
  uint16_t d_measure = 0;
  float result = 0;  
  float rx_data[TOTAL_IC];
  float tx_data[TOTAL_IC];  
  int8_t getRawTemp = BMS_TEMP_next(rx_data);
  result = convert(d_measure);
    
  /* Charging sequence with S pin PWM config */
  // Initial values in register when cleared
  static const uint8_t check = 0x00;
  // For S Pin PWM config for 28/30 second cycle
  // Set DCC bit in CRG to 1
  uint8_t dcc_bits_write[TOTAL_IC][6];
  uint8_t dcc_bits_read[TOTAL_IC][8];
  dcc_bits_write[0][4] = 255;
  // PWMC Setting 4'b1110, 93.3% Duty Cycle 
  // CFRG4[7:0] and CFRG5[3:0]
  uint8_t pwm_duty_28_sec_write[TOTAL_IC][6];
  uint8_t pwm_duty_28_sec_read[TOTAL_IC][8];
  pwm_duty_28_sec_write[0][5] = 14;

  // Combine both configs 
  static const uint8_t combined_configs[TOTAL_IC][6] = {{0x00, 0x00, 0x00, 0x00, 0xFF, 0x0E}, // <- Modifications
                                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};  // test with print rx

  // Address of CFRG4 
  LTC6804_wrcfg(TOTAL_IC, dcc_bits_write);
  if(LTC6804_rdcfg(TOTAL_IC, dcc_bits_read))
  {
    if(dcc_bits_read[0][4] == check)
    {
      Serial.println("Error! Cannot write DCC");
    }    
  }
  else 
  {
    Serial.println("Success!");
  }
  
  // Address of CFRG5 
   LTC6804_wrcfg(TOTAL_IC, pwm_duty_28_sec_write);
  if(LTC6804_rdcfg(TOTAL_IC, pwm_duty_28_sec_read))
  {
    if(pwm_duty_28_sec_read[0][5] == check)
    {
      Serial.println("Error! Cannot write PWM");
    }    
  }
  else 
  {
    Serial.println("Success!");
  }

  // Temp sensing 
  if(!getRawTemp)
  {
    Serial.println("Error! Cannot get raw temp");
    set_fault();
  }
  else
  {
    for(int current_ic = 0; current_ic < TOTAL_IC; current_ic++)
    {
      Serial.print("temp: ");
      Serial.println(result);  
    }      
  }
}