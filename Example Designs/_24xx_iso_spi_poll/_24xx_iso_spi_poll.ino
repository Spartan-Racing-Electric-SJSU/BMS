/*!
Modified LTC24xx sketch for isoSPI LTC6820


@endverbatim

REVISION HISTORY
$Revision: 6563 $
$Date: 2017-02-17 15:45:17 -0800 (Fri, 17 Feb 2017) $


Copyright (c) 2017, Linear Technology Corp.(LTC)
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

/*! @ingroup Example_Designs
    @{
    @defgroup LTC24xx_iso_spi_poll LTC24xx_iso_spi_poll: Modified LTC24xx sketch for isoSPI LTC6820
    @}
*/
/*! @file
    @ingroup LTC24xx_iso_spi_poll
*/

#include <Arduino.h>
#include <stdint.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "UserInterface.h"
#include "QuikEval_EEPROM.h"
#include "LTC24XX_general.h"
#include <SPI.h>
#include <Wire.h>
#include "LT_I2C.h"

// Function Declaration
void print_title();                             // Print the title block
void print_prompt();                            // Prompt the user for an input command
void print_user_command(uint8_t menu);             // Display selected differential channels

uint8_t menu_1_read_single_ended();
uint8_t menu_2_read_differential();

// Global variables
static uint8_t demo_board_connected;            //!< Set to 1 if the board is connected
static float reference_voltage = 5.0;             //!< The reference voltage range, set to 5v through JP2 and JP3 by default

//Constants
const uint16_t MISO_TIMEOUT = 100;       //!< The MISO timeout (ms)

// Build the command for single-ended mode
const uint8_t BUILD_COMMAND_SINGLE_ENDED[16] = {LTC24XX_MULTI_CH_CH0,
    LTC24XX_MULTI_CH_CH1,
    LTC24XX_MULTI_CH_CH2,
    LTC24XX_MULTI_CH_CH3,
    LTC24XX_MULTI_CH_CH4,
    LTC24XX_MULTI_CH_CH5,
    LTC24XX_MULTI_CH_CH6,
    LTC24XX_MULTI_CH_CH7,
    LTC24XX_MULTI_CH_CH8,
    LTC24XX_MULTI_CH_CH9,
    LTC24XX_MULTI_CH_CH10,
    LTC24XX_MULTI_CH_CH11,
    LTC24XX_MULTI_CH_CH12,
    LTC24XX_MULTI_CH_CH13,
    LTC24XX_MULTI_CH_CH14,
    LTC24XX_MULTI_CH_CH15
                                               };        //!< Builds the command for single-ended mode

// Build the command for differential mode
const uint8_t BUILD_COMMAND_DIFF[16] = {LTC24XX_MULTI_CH_P0_N1,
                                        LTC24XX_MULTI_CH_P2_N3,
                                        LTC24XX_MULTI_CH_P4_N5,
                                        LTC24XX_MULTI_CH_P6_N7,
                                        LTC24XX_MULTI_CH_P8_N9,
                                        LTC24XX_MULTI_CH_P10_N11,
                                        LTC24XX_MULTI_CH_P12_N13,
                                        LTC24XX_MULTI_CH_P14_N15,
                                        LTC24XX_MULTI_CH_P1_N0,
                                        LTC24XX_MULTI_CH_P3_N2,
                                        LTC24XX_MULTI_CH_P5_N4,
                                        LTC24XX_MULTI_CH_P7_N6,
                                        LTC24XX_MULTI_CH_P9_N8,
                                        LTC24XX_MULTI_CH_P11_N10,
                                        LTC24XX_MULTI_CH_P13_N12,
                                        LTC24XX_MULTI_CH_P15_N14
                                       };


//! Initialize Linduino
void setup()
// Setup the program
{
  quikeval_SPI_init();          // Configure the spi port for 4MHz SCK
  quikeval_SPI_connect();       // Connect SPI to main data port
  quikeval_I2C_init();          // Configure the EEPROM I2C port for 100kHz
  Serial.begin(115200);         // Initialize the serial port to the PC
  print_title();

  demo_board_connected = 1;     // Bypass EEPROM chack
  if (demo_board_connected)
  {
    print_prompt();
  }
}

//! Repeats Linduino loop
void loop()
{
  uint8_t user_command;             // The user input command
  uint8_t ack = 0;
  if (demo_board_connected)
  {
    if (Serial.available())         // Check for user input
    {
      user_command = read_int();    // Read the user command
      if (user_command == 'm');
      else
        Serial.println(user_command);
      delay(50);                    // Allow the print to finish
      switch (user_command)
      {
        case 1:
          ack |= menu_1_read_single_ended();
          break;
        case 2:
          ack |= menu_2_read_differential();
          break;
        default:
          Serial.println("Incorrect Option");
          break;
      }
      if (ack)
        Serial.println(F("***** SPI ERROR *****"));
      Serial.println(F("*****************************************************************"));
      print_prompt();
    }
  }
}

//Function Definitions

//! Prints the title block when program first starts.
void print_title()
{
  Serial.println();
  Serial.println(F("*****************************************************************"));
  Serial.println(F("*                                                               *"));
  Serial.println(F("*                                                               *"));
  Serial.println(F("* This program demonstrates how to send data and receive data   *"));
  Serial.println(F("* from the 24-bit delta-sigma ADC and LTC6820.                  *"));
  Serial.println(F("*                                                               *"));
  Serial.println(F("*                                                               *"));
  Serial.println(F("* Set the baud rate to 115200 select the newline terminator.    *"));
  Serial.println(F("*                                                               *"));
  Serial.println(F("*****************************************************************"));
}

//! Prints main menu.
void print_prompt()
{

  Serial.println(F("1-Read Single-Ended"));
  Serial.println(F("2-Read Differential"));
  Serial.println();
  Serial.print(F("Enter a command:"));
}

//! read from ADC single-ended
//! @return 0 if successful, 1 if failure
//! @return 0 when m is entered into menu, 1 if timeout for EOC
uint8_t menu_1_read_single_ended()
{
  uint8_t  adc_command;  // The LTC24XX command byte
  int16_t user_command;  // The user input command
  int32_t adc_code=0;    // The LTC24XX code
  float adc_voltage;     // The LTC24XX voltage

  while (1)
  {
    Serial.print(F("*************************\n\n"));      // Display single-ended menu
    Serial.print(F("0-CH0  8-CH8\n"));
    Serial.print(F("1-CH1  9-CH9\n"));
    Serial.print(F("2-CH2  10-CH10\n"));
    Serial.print(F("3-CH3  11-CH11\n"));
    Serial.print(F("4-CH4  12-CH12\n"));
    Serial.print(F("5-CH5  13-CH13\n"));
    Serial.print(F("6-CH6  14-CH14\n"));
    Serial.print(F("7-CH7  15-CH15\n"));
    Serial.print(F("16-ALL\n"));
    Serial.print(F("m-Main Menu\n"));
    Serial.print(F("\nEnter a Command: "));
    user_command = read_int();                              // Read the single command
    if (user_command == 'm')
      return(0);
    else
      Serial.println(user_command);
    Serial.println();

    if (user_command == 16)
    {
      Serial.print(F("ALL\n"));
      adc_command = BUILD_COMMAND_SINGLE_ENDED[0];          // Build ADC command for channel 0
      while (LTC24XX_EOC_timeout(LTC24XX_CS, MISO_TIMEOUT));
      LTC24XX_SPI_16bit_command_32bit_data(LTC24XX_CS, adc_command, 0x00, &adc_code);     // Throws out last reading
      while (LTC24XX_EOC_timeout(LTC24XX_CS, MISO_TIMEOUT));
      LTC24XX_SPI_16bit_command_32bit_data(LTC24XX_CS, adc_command, 0x00, &adc_code);     // Throws out last reading

      for (int8_t x = 0; x <= 15; x++)
      {
        adc_command = BUILD_COMMAND_SINGLE_ENDED[(x + 1) % 16]; // Read all channels in single-ended mode
        while (LTC24XX_EOC_timeout(LTC24XX_CS, MISO_TIMEOUT));
        LTC24XX_SPI_16bit_command_32bit_data(LTC24XX_CS, adc_command, 0x00, &adc_code);
        adc_voltage = LTC24XX_diff_code_to_voltage(adc_code, reference_voltage);
        Serial.print(F("  ****"));
        Serial.print(F("CH"));
        Serial.print(x);
        Serial.print(F(": "));
        Serial.print(adc_voltage, 4);
        Serial.print(F("V\n"));
      }
    }
    else if (user_command<16 && user_command >=0)                           // Read selected channel
    {
      adc_command = BUILD_COMMAND_SINGLE_ENDED[user_command];
      Serial.print(F("\nADC Command: h"));
      Serial.println(adc_command, HEX);
      while (LTC24XX_EOC_timeout(LTC24XX_CS, MISO_TIMEOUT));
      LTC24XX_SPI_16bit_command_32bit_data(LTC24XX_CS, adc_command, 0x00, &adc_code);     // Throws out last reading
      while (LTC24XX_EOC_timeout(LTC24XX_CS, MISO_TIMEOUT));
      LTC24XX_SPI_16bit_command_32bit_data(LTC24XX_CS, adc_command, 0x00, &adc_code);
      Serial.print(F("Received Code: 0x"));
      Serial.println(adc_code, HEX);
      adc_voltage = LTC24XX_diff_code_to_voltage(adc_code, reference_voltage);
      Serial.print(F("  ****"));
      Serial.print(F("CH"));
      Serial.print(user_command);
      Serial.print(F(": "));
      Serial.print(adc_voltage, 4);
      Serial.print(F("V\n"));
    }
  }
}

//! Read channels in differential mode
//! @return 0 when m is entered into menu, 1 if timeout for EOC
uint8_t menu_2_read_differential()
{
  int8_t y;             // Offset into differential channel array to select polarity
  uint8_t  adc_command; // The LTC24XX command byte
  int16_t user_command; // The user input command
  int32_t adc_code=1;   // The LTC24XX code
  float adc_voltage;    // The LTC24XX voltage

  while (1)
  {
    // Display differential menu
    Serial.print(F("\n*************************\n\n"));
    Serial.print(F("0-0P-1N     8-1P-0N\n"));
    Serial.print(F("1-2P-3N     9-3P-2N\n"));
    Serial.print(F("2-4P-5N     10-5P-4N\n"));
    Serial.print(F("3-6P-7N     11-7P-6N\n"));
    Serial.print(F("4-8P-9N     12-9P-8N\n"));
    Serial.print(F("5-10P-11N   13-11P-10N\n"));
    Serial.print(F("6-12P_13N   14-13P-12N\n"));
    Serial.print(F("7-14P-15N   15-15P-14N\n"));
    Serial.print(F("16-ALL Even_P-Odd_N\n"));
    Serial.print(F("17-ALL Odd_P-Even_N\n"));
    Serial.print(F("m-Main Menu\n"));
    Serial.print(F("\nEnter a Command: "));
    user_command = read_int();
    if (user_command == 'm')
      return(0);
    Serial.println(user_command);
    Serial.println();

    if ((user_command == 16) || (user_command == 17))
    {
      if (user_command == 16)
      {
        Serial.print(F("ALL Even_P-Odd_N\n"));              // Cycles through options 0-7
        y = 0;
        adc_command = BUILD_COMMAND_DIFF[y];
      }
      if (user_command == 17)
      {
        Serial.print(F("ALL Odd_P-Even_N\n"));              // Cycles through options 8-15
        y = 8;
        adc_command = BUILD_COMMAND_DIFF[y];
      }
      while (LTC24XX_EOC_timeout(LTC24XX_CS, MISO_TIMEOUT));
      LTC24XX_SPI_16bit_command_32bit_data(LTC24XX_CS, adc_command, 0x00, &adc_code);     // Throws out last reading
      while (LTC24XX_EOC_timeout(LTC24XX_CS, MISO_TIMEOUT));
      LTC24XX_SPI_16bit_command_32bit_data(LTC24XX_CS, adc_command, 0x00, &adc_code);     // Throws out last reading
      for (int8_t x = 0; x <= 7; x++)                       // Read all channels. All even channels are positive and odd channels are negative
      {
        adc_command = BUILD_COMMAND_DIFF[((x+1) % 8) + y];
        while (LTC24XX_EOC_timeout(LTC24XX_CS, MISO_TIMEOUT));
        LTC24XX_SPI_16bit_command_32bit_data(LTC24XX_CS, adc_command, 0x00, &adc_code);
        adc_voltage = LTC24XX_diff_code_to_voltage(adc_code, reference_voltage);
        Serial.println();
        Serial.print(F("**** "));
        print_user_command(x + y);
        Serial.print(F(": "));
        Serial.print(adc_voltage, 4);
        Serial.print(F("V\n"));
      }
    }
    else                                                    // Read selected channels
    {
      // Reads and displays a selected channel
      adc_command = BUILD_COMMAND_DIFF[user_command];
      Serial.print(F("ADC Command: 0b"));
      Serial.println(adc_command, BIN);
      while (LTC24XX_EOC_timeout(LTC24XX_CS, MISO_TIMEOUT));
      LTC24XX_SPI_16bit_command_32bit_data(LTC24XX_CS, adc_command, 0x00, &adc_code);     // Throws out last reading
      while (LTC24XX_EOC_timeout(LTC24XX_CS, MISO_TIMEOUT));
      LTC24XX_SPI_16bit_command_32bit_data(LTC24XX_CS, adc_command, 0x00, &adc_code);
      Serial.print(F("Received Code: 0x"));
      Serial.println(adc_code, HEX);
      adc_voltage = LTC24XX_diff_code_to_voltage(adc_code, reference_voltage);
      Serial.println();
      Serial.print(F("**** "));
      Serial.print(F("CH"));
      print_user_command(user_command);
      Serial.print(F(": "));
      Serial.print(adc_voltage, 4);
      Serial.print(F("V"));
      Serial.println();
    }
  }
}

//! Display selected differential channels. Displaying Single-Ended channels is
//! straightforward; not so with differential because the inputs can take either polarity.
void print_user_command(uint8_t menu) //!< the selected channels
{
  switch (menu)
  {
    case 0:
      Serial.print("0P-1N");
      break;
    case 1:
      Serial.print("2P-3N");
      break;
    case 2:
      Serial.print("4P-5N");
      break;
    case 3:
      Serial.print("6P-7N");
      break;
    case 4:
      Serial.print("8P-9N");
      break;
    case 5:
      Serial.print("10P-11N");
      break;
    case 6:
      Serial.print("12P-13N");
      break;
    case 7:
      Serial.print("14P-15N");
      break;
    case 8:
      Serial.print("1P-0N");
      break;
    case 9:
      Serial.print("3P-2N");
      break;
    case 10:
      Serial.print("5P-4N");
      break;
    case 11:
      Serial.print("7P-6N");
      break;
    case 12:
      Serial.print("9P-8N");
      break;
    case 13:
      Serial.print("11P-10N");
      break;
    case 14:
      Serial.print("13P-12N");
      break;
    case 15:
      Serial.print("15P-14N");
      break;
  }
  Serial.print(": ");
}
