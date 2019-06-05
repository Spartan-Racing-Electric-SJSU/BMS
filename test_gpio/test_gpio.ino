#include <Arduino.h>
#include <stdint.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "LT_I2C.h"
#include "UserInterface.h"
#include "LTC681x.h"
#include "LTC6811.h"
#include <SPI.h>

/* BOILERPLATE START */
#define ENABLED 1
#define DISABLED 0

#define DATALOG_ENABLED 1
#define DATALOG_DISABLED 0

const uint8_t TOTAL_IC = 1;

 //ADC Command Configurations
const uint8_t ADC_OPT = ADC_OPT_DISABLED; // See LTC6811_daisy.h for Options
const uint8_t ADC_CONVERSION_MODE = MD_7KHZ_3KHZ;//MD_7KHZ_3KHZ; //MD_26HZ_2KHZ;//MD_7KHZ_3KHZ; // See LTC6811_daisy.h for Options
const uint8_t ADC_DCP = DCP_DISABLED; // See LTC6811_daisy.h for Options
const uint8_t CELL_CH_TO_CONVERT = CELL_CH_ALL; // See LTC6811_daisy.h for Options
const uint8_t AUX_CH_TO_CONVERT = AUX_CH_ALL; // See LTC6811_daisy.h for Options
const uint8_t STAT_CH_TO_CONVERT = STAT_CH_ALL; // See LTC6811_daisy.h for Options

const uint16_t MEASUREMENT_LOOP_TIME = 500;//milliseconds(mS)

//Under Voltage and Over Voltage Thresholds
const uint16_t OV_THRESHOLD = 41000; // Over voltage threshold ADC Code. LSB = 0.0001
const uint16_t UV_THRESHOLD = 30000; // Under voltage threshold ADC Code. LSB = 0.0001

//Loop Measurement Setup These Variables are ENABLED or DISABLED Remember ALL CAPS
const uint8_t WRITE_CONFIG = DISABLED; // This is ENABLED or DISABLED
const uint8_t READ_CONFIG = DISABLED; // This is ENABLED or DISABLED
const uint8_t MEASURE_CELL = ENABLED; // This is ENABLED or DISABLED
const uint8_t MEASURE_AUX = DISABLED; // This is ENABLED or DISABLED
const uint8_t MEASURE_STAT = DISABLED; //This is ENABLED or DISABLED
const uint8_t PRINT_PEC = DISABLED; //This is ENABLED or DISABLED
/* BOILERPLATE END */

void setup()
{
  Serial.begin(115200);
  quikeval_SPI_connect();
  spi_enable(SPI_CLOCK_DIV32); // This will set the Linduino to have a 1MHz Clock
  quikeval_I2C_init();

  //TODO: This can be simplified, hardcoded for now
  LTC681x_init_cfg(TOTAL_IC, bms_ic);
  LTC6811_reset_crc_count(TOTAL_IC,bms_ic);
  LTC6811_init_reg_limits(TOTAL_IC,bms_ic);
  LTC681x_init_cfg(TOTAL_IC, bms_ic24);
  LTC6811_reset_crc_count(TOTAL_IC,bms_ic24);
  LTC6811_init_reg_limits(TOTAL_IC,bms_ic24);
  LTC681x_init_cfg(TOTAL_IC, bms_ic48);
  LTC6811_reset_crc_count(TOTAL_IC,bms_ic48);
  LTC6811_init_reg_limits(TOTAL_IC,bms_ic48);
  LTC681x_init_cfg(TOTAL_IC, read_bms_ic24);
  LTC6811_reset_crc_count(TOTAL_IC,read_bms_ic24);
  LTC6811_init_reg_limits(TOTAL_IC,read_bms_ic24);
  LTC681x_init_cfg(TOTAL_IC, read_bms_ic48);
  LTC6811_reset_crc_count(TOTAL_IC,read_bms_ic48);
  LTC6811_init_reg_limits(TOTAL_IC,read_bms_ic48);
}

void print_data(uint8_t num_ic, cell_asic * ic) 
{
  for(index=0;index<num_ic;index++)
  {
    Serial.print("data ");
    Serial.print(index);
    Serial.print(":\t");
    Serial.println(ic[0].com.rx_data[index]);
  }
}

void loop()
{
  // Local Variables
  bool gpio_reg[5] = {1,1,1,1,1};

  cell_asic bms_ic24[TOTAL_IC];
  cell_asic bms_ic48[TOTAL_IC];
  cell_asic read_bms_ic24[TOTAL_IC];
  cell_asic read_bms_ic48[TOTAL_IC];
  cell_asic bms_ic[TOTAL_IC];

  // PEC calc 
  uint8_t * pec_data;
  uint16_t pec_temp = pec15_calc(TOTAL_IC, pec_data);

  // setup GPIO registers
  LTC681x_set_cfgr_gpio(TOTAL_IC,bms_ic,gpio_reg);
  LTC681x_set_cfgr_gpio(TOTAL_IC,bms_ic24,gpio_reg);
  LTC681x_set_cfgr_gpio(TOTAL_IC,bms_ic48,gpio_reg);

  //TODO: Organize this into a loop after test
  // Read IC 0 cfg
  bms_ic[0].com.tx_data[0] = 0x62;
  bms_ic[0].com.tx_data[1] = 0x98;
  bms_ic[0].com.tx_data[2] = 0x0f;
  bms_ic[0].com.tx_data[3] = 0xf0;
  bms_ic[0].com.tx_data[4] = 0x0f;
  bms_ic[0].com.tx_data[5] = 0xf9;

  // Read IC 1 cfg
  bms_ic[1].com.tx_data[0] = 0x62;
  bms_ic[1].com.tx_data[1] = 0x98;
  bms_ic[1].com.tx_data[2] = 0x0f;
  bms_ic[1].com.tx_data[3] = 0xf0;
  bms_ic[1].com.tx_data[4] = 0x0f;
  bms_ic[1].com.tx_data[5] = 0xf9;

  //https://ez.analog.com/power/f/q-a/110060/ltc6811-communication-through-i2c/334332
  bms_ic24[0].com.tx_data[0] = B01100010; //ICOM0,D0
  bms_ic24[0].com.tx_data[1] = B10000000; //D0,FCOM0  
  bms_ic24[0].com.tx_data[2] = B00001111; //ICOM1,D1
  bms_ic24[0].com.tx_data[3] = B11111000; //D1,FCOM1
  bms_ic24[0].com.tx_data[4] = B00001111; //ICOM2,D2
  bms_ic24[0].com.tx_data[5] = B11111001; //D2,FCOM2

  bms_ic48[0].com.tx_data[0] = B01101111; //ICOM0,D0
  bms_ic48[0].com.tx_data[1] = B11110000; //D0,FCOM0
  bms_ic48[0].com.tx_data[2] = B01101111; //ICOM1,D1
  bms_ic48[0].com.tx_data[3] = B11110000; //D1,FCOM1   
  bms_ic48[0].com.tx_data[4] = B10011111; //ICOM2,D2
  bms_ic48[0].com.tx_data[5] = B11111001; //D2,FCOM2
  
  // Redundant wakeup
  wakeup_sleep(TOTAL_IC);
 
  //Read IC 0 of i2c bus  
  Serial.println("read ic0");
  LTC6811_wrcomm(TOTAL_IC,bms_ic);  
  LTC681x_stcomm();
  //READ PEC, send 24 clocks  
  LTC6811_rdcomm(TOTAL_IC,bms_ic24);  
  LTC681x_stcomm();
  LTC6811_rdcfg(TOTAL_IC,read_bms_ic24);
  Serial.print("read 24:\t");
  print_data(TOTAL_IC,read_bms_ic24);
  //READ PEC, send 48 clocks    
  // LTC6811_rdcomm(TOTAL_IC,bms_ic48);  
  // LTC681x_stcomm();
  // LTC6811_rdcfg(TOTAL_IC,read_bms_ic48);
  // Serial.print("read 48:\t");
  // print_data(TOTAL_IC,read_bms_ic48);
}