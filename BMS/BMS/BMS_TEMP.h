// BMS_TEMP.cpp
// Temperature Calculations and ADC for the BMS
// Authors: Alex Sternberg
// 			Thinh Le
// Property of Spartan Racing Electric 2018
#ifndef _BMS_TEMP_H
#define _BMS_TEMP_H

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
#include <stdint.h>

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

#define TEMP_READ_COMMAND { 0x62, 0x88, 0x00, 0x18, 0x0A, 0xA9 }
int8_t BMS_TEMP_NEXT(float rx_temp[TOTAL_IC]);
float convert(uint16_t measure);

/**
 * Under/Over Voltage Monitor
 * parameters : C inputs
 * return : • compare to under/overvoltage thresholds 
 *              - stored in Config Register Group
 *          • flags set if condition true 
 *              - stored in Status Register Group B
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * ADAX Commands
 * parameters : • GPIO1-5 input 
 *                  - All 5 at once or 1
 *              • ADC mode
 *              • 2nd ref
 * return : • read external temperatures from sensor to GPIO
 *          • 2nd reference (powers sensor) for ratiometric
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * ADAXD Commands
 * Same as ADAX but has digital redundancy
 * return : • Mismatch sends 0xFF0X to result register 
 *          • Outside clamping range of ADC, host indicates fault
 *          • Last 4 bits indicate Mismatch
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * ADCVAX Command 
 * parameters : • 12 cell measurements
 *              • 2 GPIO measurements (GPIO1 and GPIO2)
 * return : sync battery cell voltage and current measurements  
 *              - When current sensor connected to GPIO 1 or 2
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * DAQ
 * parameters : • Muxes
 *              • ADC
 *              • 1st reference 
 *              • Digital filters
 *              • Memory 
 * return : Circuit verification
 *          • Sum of all Cells (SC)
 *          • Internal Die Temperature (ITMP)
 *          • Analog Power Supply (VA)
 *          • Digital Power Supply (VD)
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Discharge
 * • Thermal shutdown
 *  - Die detects above 150 C 
 *  - Shutdown circuit resets Config Reg Group and S Control Reg to default states
 * • Watchdog (WDT) 
 *  - Timer is 2 seconds and expires if no valid command 
 *  - SCR is reset by watchdog timer if discharge timer disabled
 *  - WDT elapsed, WDT pin is pulled high by external PU 
 *  - DT keep discharge switches ON for time duration
 *  - If DT used, switches not turned OFF when WDT activated
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * S Pin PWM for Cell Balancing
 * • After WDT expires, PWM begins 
 *  - Continues until remainder of discharge time or wake-up event
 * • DCC bits must be set to 1
 * • May cause S pins to periodically de-asset for duty cycle (30 seconds)
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * I2C and SPI Master
 * • I/O ports GPIO3,4,5 for I2C or SPI Slave
 * • I2C    
 *  - GPIO4 and 5 form SDA and SCL
 * • SPI
 *  - GPIO3 is CSBM
 *  - GPIO4 is SDIOM
 *  - GPIO5 is SCKM
 * • GPIO are open drain 
 *  - Require external PU 
 *  - Set GPIO bits to 1 in Config Reg so no pulled low internally
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * COMM Register
 * • Stores all data and control bits required for I2C or SPI communication
 * • Dn[7:0] to be sent/receive from slave 
 * • ICOMn[3:0] controls before transceiving data   
 *  - ICOMn[3] = 1, SPI 
 *  - ICOMn[3] = 0, I2C
 * • FCOMn[3:0] control after transceiving data 
 *  - ACK or NACK, CSBM high or low 
 * • WRCOMM 
 *  - Writes 6 bytes of data to COMM, PEC(Packet Error Code) at the end 
 * • STCOMM 
 *  - initiates I2C/SPI on GPIO ports 
 *  - 3 bytes of data to slave 
 * • RDCOMM
 *  - Reads 6 bytes of data followed by PEC 
 */

#endif 