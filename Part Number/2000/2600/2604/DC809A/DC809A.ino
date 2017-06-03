/*!
LTC2604: Quad 16-Bit Rail-to-Rail DACs in 16-Lead SSOP

@verbatim

The LTC2604/LTC2614/LTC2624 are quad 16-,14- and 12-bit 2.5V to 5.5V
rail-to-rail voltage output DACs in 16-lead narrow SSOP packages. These
parts have separate reference inputs for each DAC. They have built-in
high performance output buffers and are guaranteed monotonic.

These parts establish advanced performance standards for output drive,
crosstalk and load regulation in single-supply, voltage output multiples.

@endverbatim


http://www.linear.com/product/LTC2604

http://www.linear.com/product/LTC2604#demoboards

REVISION HISTORY
$Revision: 5670 $
$Date: 2016-09-02 10:55:41 -0700 (Fri, 02 Sep 2016) $

Copyright (c) 2014, Linear Technology Corp.(LTC)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of Linear Technology Corp.

The Linear Technology Linduino is not affiliated with the official Arduino team.
However, the Linduino is only possible because of the Arduino team's commitment
to the open-source community.  Please, visit http://www.arduino.cc and
http://store.arduino.cc , and consider a purchase that will help fund their
ongoing work.
*/


/*! @file
  @ingroup LTC2604
*/

#include <Arduino.h>
#include <stdint.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "UserInterface.h"
#include "LT_I2C.h"
#include "QuikEval_EEPROM.h"
#include "LTC2604.h"
#include <SPI.h>
#include <Wire.h>

//Function Declaration

void print_title();                         // Print the title block
void print_prompt(int16_t selected_dac);    // Prompt the user for an input command
int16_t prompt_voltage_or_code();
uint16_t get_voltage(float LTC2604_lsb, int16_t LTC2604_offset);
uint16_t get_code();

void menu_1_select_dac(int16_t *selected_dac);
void menu_2_write_to_input_register(int16_t selected_dac);
void menu_3_write_and_update_dac(int16_t selected_dac);
void menu_4_update_power_up_dac(int16_t selected_dac);
void menu_5_power_down_dac(int16_t selected_dac);

//Global variables
static uint8_t demo_board_connected; //!< set to 1 if the board is connected
static uint8_t shift_count =0;       //!< The data align shift count for 16-bit =0 14-bit =2 12-bit =4
static float reference_voltage[4] = {4.096,4.096,4.096,4.096}; //!< set based on jumper positions
static int16_t LTC2604_offset =0;
static float LTC2604_lsb[4] = {reference_voltage[0]/ (pow(2,16) - 1) , reference_voltage[1]/ (pow(2,16) - 1),
                               reference_voltage[2]/ (pow(2,16) - 1),reference_voltage[3]/ (pow(2,16) - 1)
                              };

// constants

//! Lookup table for DAC address. Allows the "All DACs" address to be indexed right after DAC D in loops.
//! This technique is very useful for devices with non-monotonic channel addresses.
const uint8_t address_map[5] = {LTC2604_DAC_A, LTC2604_DAC_B, LTC2604_DAC_C, LTC2604_DAC_D, LTC2604_DAC_ALL};  //!< Map entered option 0..2 to DAC address

//! Used to keep track to print voltage or print code
enum
{
  PROMPT_VOLTAGE = 0, /**< 0 */
  PROMPT_CODE = 1     /**< 1 */
};

//! Initialize Linduino
void setup()
// Setup the program
{
  char demo_name[] = "DC809A";  // Demo Board Name stored in QuikEval EEPROM
  quikeval_SPI_init();          // Configure the spi port for 4MHz SCK
  quikeval_SPI_connect();       // Connect SPI to main data port
  Serial.begin(115200);         // Initialize the serial port to the PC
  print_title();
  demo_board_connected = true; //discover_demo_board(demo_name);
  if (demo_board_connected)
  {
    print_prompt(0);
  }
}

//! Repeats Linduino loops
void loop()
{
  int16_t user_command;
  static int16_t selected_dac =0;      // The selected DAC to be updated (0=A, 1=B ... 5=All).  Initialized to "A".
  // The main control loop
  if (demo_board_connected)             // Do nothing if the demo board is not connected
  {
    if (Serial.available())             // Check for user input
    {
      user_command = read_int();        // Read the user command
      Serial.println(user_command);
      Serial.flush();
      switch (user_command)
      {
        case 1:
          menu_1_select_dac(&selected_dac);
          break;
        case 2:
          menu_2_write_to_input_register(selected_dac);
          break;
        case 3:
          menu_3_write_and_update_dac(selected_dac);
          break;
        case 4:
          menu_4_update_power_up_dac(selected_dac);
          break;
        case 5:
          menu_5_power_down_dac(selected_dac);
          break;
      }
      Serial.println("\n*****************************************************************");
      print_prompt(selected_dac);
    }
  }
}

// Function Definitions

//! Select which DAC to operate on
void menu_1_select_dac(int16_t *selected_dac ) //!< what DAC to operate on
{
  // Select a DAC to operate on
  Serial.print("Select DAC to operate on (0=A, 1=B, 2=C, 3=D, 4=All)");
  *selected_dac = read_int();
  if (*selected_dac == 4)
    Serial.println("All");
  else
    Serial.println(*selected_dac);
}

//! Write data to input register, but do not update DAC output
void menu_2_write_to_input_register(int16_t selected_dac) //!< what DAC to operate on
{
  uint16_t dac_code;

  if (prompt_voltage_or_code() == PROMPT_VOLTAGE)
    dac_code = get_voltage(LTC2604_lsb[selected_dac], LTC2604_offset);
  else
    dac_code = get_code();

  LTC2604_write(LTC2604_CS, LTC2604_CMD_WRITE, address_map[selected_dac], dac_code << shift_count);
}

//!Write data to DAC register (which updates output immediately)
void menu_3_write_and_update_dac(int16_t selected_dac) //!< what DAC to operate on
{
  uint16_t dac_code;

  if (prompt_voltage_or_code() == PROMPT_VOLTAGE)
    dac_code = get_voltage(LTC2604_lsb[selected_dac], LTC2604_offset);
  else
    dac_code = get_code();

  LTC2604_write(LTC2604_CS,LTC2604_CMD_WRITE_UPDATE, address_map[selected_dac], dac_code << shift_count);
}

//! Update DAC with data that is stored in input register, power up if sleeping
void menu_4_update_power_up_dac(int16_t selected_dac) //!< what DAC to operate on
{
  // Update DAC
  LTC2604_write(LTC2604_CS,LTC2604_CMD_UPDATE, address_map[selected_dac], 0x0000);
}

//! Power down DAC
void menu_5_power_down_dac(int16_t selected_dac) //!< what DAC to operate on
{
  // Power down DAC
  LTC2604_write(LTC2604_CS,LTC2604_CMD_POWER_DOWN, address_map[selected_dac], 0x0000);
}


//! Prompt user to enter a voltage or digital code to send to DAC
//! @returns user input
int16_t prompt_voltage_or_code()
{
  int16_t user_input;
  Serial.print(F("Type 1 to enter voltage, 2 to enter code:"));
  Serial.flush();
  user_input = read_int();
  Serial.println(user_input);

  if (user_input != 2)
    return(PROMPT_VOLTAGE);
  else
    return(PROMPT_CODE);
}

//! Get voltage from user input, calculate DAC code based on lsb, offset
//! @returns the DAC code
uint16_t get_voltage(float LTC2604_lsb, //!< the voltage LSB of the DAC
                     int16_t LTC2604_offset) //!< the Offset of the DAC
{
  float dac_voltage;

  Serial.print(F("Enter Desired DAC output voltage: "));
  dac_voltage = read_float();
  Serial.print(dac_voltage);
  Serial.println(" V");
  Serial.flush();
  return(LTC2604_voltage_to_code(dac_voltage, LTC2604_lsb, LTC2604_offset));
}

//! Get code to send to DAC directly, in decimal, hex, or binary
//! @return code from user
uint16_t get_code()
{
  uint16_t returncode;
  Serial.println("Enter Desired DAC Code");
  Serial.print("(Format 32768, 0x8000, 0100000, or B1000000000000000): ");
  returncode = (uint16_t) read_int();
  Serial.print("0x");
  Serial.println(returncode, HEX);
  Serial.flush();
  return(returncode);
}

//! Prints the title block when program first starts.
void print_title()
{
  Serial.println("");
  Serial.println(F("*****************************************************************"));
  Serial.println(F("* DC809A Demonstration Program                                  *"));
  Serial.println(F("*                                                               *"));
  Serial.println(F("* This program demonstrates how to send data to the LTC2604     *"));
  Serial.println(F("* quad 16/14/12-bit DAC found on the DC809A demo board.         *"));
  Serial.println(F("*                                                               *"));
  Serial.println(F("* Set the baud rate to 115200 and select the newline terminator.*"));
  Serial.println(F("*                                                               *"));
  Serial.println(F("*****************************************************************"));
}

//! Prints main menu.
void print_prompt(int16_t selected_dac) //!< what DAC to operate on
{
  Serial.println(F("\nCommand Summary:"));
  Serial.println(F("  1-Select DAC"));
  Serial.println(F("  2-Write to input register (no update)"));
  Serial.println(F("  3-Write and update DAC"));
  Serial.println(F("  4-Update / power up DAC"));
  Serial.println(F("  5-Power down DAC"));

  Serial.println("\nPresent Values:");
  Serial.print("  Selected DAC: ");
  if (selected_dac != 4)
    Serial.println((char) (selected_dac + 0x41));
  else
    Serial.println("All");
  Serial.print("  DAC Reference: ");
  Serial.println("Internal");
  Serial.print(F("Enter a command:"));

  Serial.flush();
}