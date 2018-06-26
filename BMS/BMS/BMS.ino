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

void setup() {
  Serial.begin(115200);  

  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH); //setup fault
  
  // spi_enable(SPI_CLOCK_DIV16);
  // set_adc(MD_NORMAL, DCP_DISABLED, CELL_CH_ALL, AUX_CH_ALL);
  LTC6804_initialize();  //Initialize LTC6804 hardware

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
    Serial.println("Error!");
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
        Serial.println("Out of range");
      }
      if (!(i % 5)) i++; // If not 5th iterstion continue
      if (i == 11) i++; // Skip channel 11
    }
  }  
}

void set_fault() {
  // Set pin 6 (CS) active low, BITCLR
  digitalWrite(6, LOW); // Sent to BMS Master Fault pin 
}

float read_current(uint8_t sensor) {
  // Definition in wiring_analog.c 
  int measure = analogRead(sensor);
  return 200 * (measure / 820);
}

#if (DEBUG)

void print_txdata(uint8_t tx_cfg[][6])
{
  Serial.println("Sent Bytes ");
  for (int current_ic = 0; current_ic<TOTAL_IC; current_ic++)
  {
    Serial.print(" IC ");
    Serial.print(current_ic + 1, DEC);
    Serial.print(": 0x");
    serial_print_hex(tx_cfg[current_ic][0]);
    Serial.print(", 0x");
    serial_print_hex(tx_cfg[current_ic][1]);
    Serial.print(", 0x");
    serial_print_hex(tx_cfg[current_ic][2]);
    Serial.print(", 0x");
    serial_print_hex(tx_cfg[current_ic][3]);
    Serial.print(", 0x");
    serial_print_hex(tx_cfg[current_ic][4]);
    Serial.print(", 0x");
    serial_print_hex(tx_cfg[current_ic][5]);
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
  if (data < 16)
  {
    Serial.print("0");
    Serial.print((byte)data, HEX);
  }
  else
    Serial.print((byte)data, HEX);
}

#endif

// Our execution of functions
void loop() {
  char input;
  // 1. Initiate state machines
  // 2. Verify hardware for SCK, CSB, ISOMD, WDT, DRIVE 
  // 3. Get data transmission going 
  // 4. Read cell voltages
  // Serial.println("Test");
  uint16_t cells[TOTAL_IC][TOTAL_LTC_CHANNEL];
  uint16_t aux[TOTAL_IC][6];
  uint8_t* pec_data; // Store pec calcs here
  wakeup_sleep(); // Sleep
  Serial.print(0);
  // BMS Declaration and Constructor
  // BatteryManagementSystem* bms = BatteryManagementSystem_new();  
  do
  {
    // input = Serial.read();
    LTC6804_initialize(); // Standby 
    // Serial.println(1); // debug #1
    LTC6804_adcv(); // Measure 
    // Serial.println(2); // debug #2    
    // Serial.println("cell voltage");
    LTC6804_rdcv(0, TOTAL_IC, cells); // Reads all IC registers and stores into 2D matrix
    for(int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
      for(int current_cell = 0; current_cell < 8; current_cell++) {
        if(current_cell == 6)
        {
          uint16_t pec_temp = pec15_calc(2, pec_data);
          cells[current_ic][current_cell] = pec_temp;
          cells[current_ic][current_cell++] = (pec_temp << 8);
        }
        // Serial.println(cells[current_ic][current_cell]);
      }
    }
    LTC6804_clrcell();
    // Serial.println(3); // debug #3
    // Serial.println("aux registers");
    LTC6804_rdaux(0, TOTAL_IC, aux); // Reads all IC GPIO and stores into 2D matrix
    for(int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
      for(int current_cell = 0; current_cell < 6; current_cell++) {
        // Serial.println(aux[current_ic][current_cell]);
      }
    }    
    LTC6804_clraux();
    // Serial.println(4); // debug #4
    // TODO: Write Configuration A for REFON 
    uint8_t wrcfga[TOTAL_IC][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
                                   {0x00, 0x00, 0x00, 0x00, 0x01, 0x00},
                                   {0x00, 0x00, 0x00, 0x01, 0x00, 0x00},
                                   {0x00, 0x00, 0x01, 0x00, 0x00, 0x00},
                                   {0x00, 0x01, 0x00, 0x00, 0x00, 0x00},
                                   {0x01, 0x00, 0x00, 0x00, 0x00, 0x00},
                                   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                                   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    uint8_t read_cells[TOTAL_IC][6];                                   
    uint8_t rx_cfg1[TOTAL_IC][8] = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};  // test with print rx                                   
    uint8_t rx_cfg2[TOTAL_IC][8] = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};  // test with print rx                                   
    // Serial.println("before write");
    LTC6804_rdcfg(TOTAL_IC, rx_cfg1);    
    // print_rxdata(rx_cfg1);
    // Serial.println("write");
    LTC6804_wrcfg(TOTAL_IC, wrcfga);
    // print_txdata(wrcfga); // REFUP
    // Serial.println("after write");
    LTC6804_rdcfg(TOTAL_IC, rx_cfg2);
    // print_rxdata(rx_cfg2);

    // read_cells(bms->voltages); // TODO        
    // // read_current(current);
    // // test 
    // print_txdata(bms->tx_cfg);
    // print_rxdata(bms->rx_cfg);    

    /* Test */
    // uint8_t *store_data; 
    // int size = 0;
    // //This will guarantee that the LTC6804 isoSPI port is awake.
    // wakeup_sleep(); // TODO 
    // bms->wakeup_sleep = true; 
    // // // WRCFGA: 00000000001
    // // LTC6804_wrcfg(TOTAL_IC, bms->tx_cfg);
    // // Read register A voltage 
    // LTC6804_rdcv_reg(1, TOTAL_IC, store_data);
    // size = sizeof(store_data) / sizeof(uint8_t);
    // for(int i = 0; i < size; i++)
    // {
    //   if(store_data[i] != NULL)
    //   {
    //     Serial.println(store_data[i]);
    //   }
    //   else{
    //     Serial.println("NULL Terminated");
    //   }
    // }
  } while(input != 'q');
}


/*!***********************************
\brief Initializes the configuration array
**************************************/
void init_cfg()
{
  uint8_t tx_cfg[TOTAL_IC][6] = {{0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00},
                                 {0xF8, 0x19, 0x16, 0xA4, 0x00, 0x00},
                                 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
                  
  LTC6804_wrcfg(TOTAL_IC, tx_cfg);
}
