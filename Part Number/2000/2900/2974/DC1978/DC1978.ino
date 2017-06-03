/*!
Linear Technology DC1978A Demonstration System (2-Board Set)
DC1809 - LTC2974 Board (manager)
DC1810 - LTM4620 Board (4 rails)

@verbatim

NOTES
  Setup:
   Set the terminal baud rate to 115200 and select the newline terminator.

@endverbatim

http://www.linear.com/product/LTC2974

http://www.linear.com/demo/DC1978

REVISION HISTORY
$Revision: 4080 $
$Date: 2015-09-24 16:36:34 -0600 (Thu, 24 Sep 2015) $

Copyright (c) 2016, Linear Technology Corp.(LTC)
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
    @ingroup LTC2974
*/

#include <Arduino.h>
#include <stdint.h>
#include "Linduino.h"
#include "UserInterface.h"
#include "LT_I2CBus.h"
#include "LT_SMBusNoPec.h"
#include "LT_SMBusPec.h"
#include "LT_PMBusMath.h"
#include "LT_PMBus.h"

#define LTC2974_I2C_ADDRESS 0x5C

// Global variables
static uint8_t ltc2974_i2c_address;
static LT_PMBusMath *math = new LT_PMBusMath();
static LT_SMBus *smbus = new LT_SMBusPec();
static LT_PMBus *pmbus = new LT_PMBus(smbus);

//! Initialize Linduino
//! @return void
void setup()
{
  Serial.begin(115200);         //! Initialize the serial port to the PC
  print_title();
  ltc2974_i2c_address = LTC2974_I2C_ADDRESS;
  print_prompt();
}

//! Repeats Linduino loop
//! @return void
void loop()
{
  uint8_t user_command;
  uint8_t res;
  uint8_t model[7];
  uint8_t revision[10];
  uint8_t *addresses = NULL;

  if (Serial.available())                          //! Checks for user input
  {
    user_command = read_int();                     //! Reads the user command
    if (user_command != 'm')
      Serial.println(user_command);

    switch (user_command)                          //! Prints the appropriate submenu
    {
      case 1:
        menu_1_basic_commands();                 // Print single-ended voltage menu
        break;
      case 2:
        menu_2_vid_commands();                   // Print VID menu
        break;
      case 3:
        menu_3_vid_commands();                   // Print VID menu
        break;
      case 4:
        pmbus->enablePec(ltc2974_i2c_address);
        delete smbus;
        delete pmbus;
        smbus = new LT_SMBusPec();
        pmbus = new LT_PMBus(smbus);
        break;
      case 5:
        pmbus->disablePec(ltc2974_i2c_address);
        delete smbus;
        delete pmbus;
        smbus = new LT_SMBusNoPec();
        pmbus = new LT_PMBus(smbus);
        break;
      case 6:
        addresses = smbus->probe(0);
        while (*addresses != 0)
        {
          Serial.print(F("ADDR 0x"));
          Serial.println(*addresses++, HEX);
        }
        break;
      case 7 :
        pmbus->startGroupProtocol();
        pmbus->restoreFromNvm(ltc2974_i2c_address);
        pmbus->executeGroupProtocol();
        break;
      default:
        Serial.println(F("Incorrect Option"));
        break;
    }
    print_prompt();
  }

}

// Function Definitions

//! Prints the title block when program first starts.
//! @return void
void print_title()
{
  Serial.print(F("\n*****************************************************************\n"));
  Serial.print(F("* DC1978A Hello World Demonstration Program                     *\n"));
  Serial.print(F("*                                                               *\n"));
  Serial.print(F("* This program demonstrates how to send and receive data from   *\n"));
  Serial.print(F("* the LTC2974 on the demo board.                                *\n"));
  Serial.print(F("*                                                               *\n"));
  Serial.print(F("* Set the baud rate to 115200 and select the newline terminator.*\n"));
  Serial.print(F("*                                                               *\n"));
  Serial.print(F("*****************************************************************\n"));
}

//! Prints main menu.
//! @return void
void print_prompt()
{
  Serial.print(F("\n  1-Basic Commands\n"));
  Serial.print(F("  2-VID Commands (L16)\n"));
  Serial.print(F("  3-VID Commands (float) \n"));
  Serial.print(F("  4-PEC On\n"));
  Serial.print(F("  5-PEC Off\n"));
  Serial.print(F("  6-Bus Probe\n"));
  Serial.print(F("  7-Reset\n"));
  Serial.print(F("\nEnter a command:"));
}

//! Prints a warning if the demo board is not detected.
//! @return void
void print_warning_prompt()
{
  Serial.println(F("\nWarning: Demo board not detected. Linduino will attempt to proceed."));
}

//! Prints all voltages
//! @return void
void print_all_voltages()
{
  float   voltage;
  uint8_t page;

  for (page = 0; page < 4; page++)
  {
    pmbus->setPage(ltc2974_i2c_address, page);
    voltage = pmbus->readVout(ltc2974_i2c_address, false);
    Serial.print(F("LTC2974 VOUT "));
    Serial.println(voltage, DEC);
  }
}

//! Prints all currents
//! @return void
void print_all_currents()
{
  float   current;
  uint8_t page;

  for (page = 0; page < 4; page++)
  {
    pmbus->setPage(ltc2974_i2c_address, page);
    current = pmbus->readIout(ltc2974_i2c_address, false);
    Serial.print(F("LTC2974 IOUT "));
    Serial.println(current, DEC);
  }
}

//! Prints all status bytes and words
//! @return void
void print_all_status()
{
  uint8_t b;
  uint16_t w;
  uint8_t page;

  for (page = 0; page < 4; page++)
  {
    Serial.print(F("PAGE "));
    Serial.println(page, DEC);
    pmbus->setPage(ltc2974_i2c_address, page);
    b = pmbus->readStatusByte(ltc2974_i2c_address);
    Serial.print(F("LTC2974 STATUS BYTE 0x"));
    Serial.println(b, HEX);
    w = pmbus->readStatusWord(ltc2974_i2c_address);
    Serial.print(F("LTC2974 STATUS WORD 0x"));
    Serial.println(w, HEX);
  }
}

// Sequence off then on
//! @return void
void sequence_off_on()
{
  pmbus->sequenceOffGlobal();
  delay (2000);
  pmbus->sequenceOnGlobal();
}

//! Margin high
//! @return void
void margin_high()
{
  pmbus->marginHighGlobal();
}

//! Margin low
//! @return void
void margin_low()
{
  pmbus->marginLowGlobal();
}

//! Go to nominal
//! @return void
void margin_off()
{
  pmbus->sequenceOnGlobal();
}

void increment_page()
{
  uint8_t reg_page;
  reg_page = smbus->readByte (ltc2974_i2c_address, 0x00);      // Read current page number
  reg_page = (reg_page < 0x02) ? reg_page + 0x01 : 0x00 ;
  smbus->writeByte (ltc2974_i2c_address, 0x00, reg_page);      // Write current page number
  reg_page = smbus->readByte (ltc2974_i2c_address, 0x00);      // Read current page number
  Serial.print(F("LTC2974 PAGE SET TO : "));
  Serial.println(reg_page, HEX);

}

//! Display menu 1
//! @return void
void menu_1_basic_commands()
{
  uint8_t user_command;

  do
  {
    //! Displays the Read/Write menu
    Serial.print(F("  1-Read All Voltages\n"));
    Serial.print(F("  2-Read All Currents\n"));
    Serial.print(F("  3-Read All Status\n"));
    Serial.print(F("  4-Sequence Off/On\n"));
    Serial.print(F("  5-Margin High\n"));
    Serial.print(F("  6-Margin Low\n"));
    Serial.print(F("  7-Margin Off\n"));
    Serial.print(F("  8-Increment Page Number\n"));
    Serial.print(F("  m-Main Menu\n"));
    Serial.print(F("\nEnter a command: "));

    user_command = read_int();                              //! Reads the user command
    if (user_command == 'm')                                // Print m if it is entered
    {
      Serial.print(F("m\n"));
    }
    else
      Serial.println(user_command);                         // Print user command

    switch (user_command)
    {
      case 1:
        print_all_voltages();
        break;
      case 2:
        print_all_currents();
        break;
      case 3:
        print_all_status();
        break;
      case 4:
        sequence_off_on();
        break;
      case 5:
        margin_high();
        break;
      case 6:
        margin_low();
        break;
      case 7:
        margin_off();
        break;
      case 8:
        increment_page();
        break;
      default:
        if (user_command != 'm')
          Serial.println(F("Invalid Selection"));
        break;
    }
  }
  while (user_command != 'm');
}

//! Display menu 2
//! @return void
void menu_2_vid_commands()
{
  uint8_t user_command;
  uint16_t vcode;
  float voltage;

  voltage = pmbus->readVout(ltc2974_i2c_address, false);
  vcode = math->float_to_lin16 (voltage, 0x13);

  do
  {
    //! Displays the Read/Write menu
    Serial.print(F("  1-VID UP 40 LSB\n"));
    Serial.print(F("  2-VID DN 40 LSB\n"));
    Serial.print(F("  m-Main Menu\n"));
    Serial.print(F("\nEnter a command: "));

    user_command = read_int();                              //! Reads the user command
    if (user_command == 'm')                                // Print m if it is entered
    {
      Serial.print(F("m\n"));
    }
    else
      Serial.println(user_command);                         // Print user command

    switch (user_command)
    {
      case 1:
        vcode += 40;
        smbus->writeWord (ltc2974_i2c_address, 0x21, vcode);
        Serial.print(F("LTC2974 VOUT Code "));
        Serial.println(vcode, HEX);
        delay (1000);
        voltage = pmbus->readVout(ltc2974_i2c_address, false);
        Serial.print(F("LTC2974 VOUT Telemetry "));
        Serial.println(voltage, DEC);
        break;
      case 2:
        vcode -= 40;
        smbus->writeWord (ltc2974_i2c_address, 0x21, vcode);
        Serial.print(F("LTC2974 VOUT Code "));
        Serial.println(vcode, HEX);
        delay (1000);
        voltage = pmbus->readVout(ltc2974_i2c_address, false);
        Serial.print(F("LTC2974 VOUT Telemetry "));
        Serial.println(voltage, DEC);
        Serial.print(F("\n"));
        break;
      default:
        if (user_command != 'm')
          Serial.println(F("Invalid Selection"));
        break;
    }
  }
  while (user_command != 'm');
}

//! Display menu 3
void menu_3_vid_commands()
{
  uint8_t user_command;
  float vid_value;
  float voltage;

  vid_value = pmbus->readVout(ltc2974_i2c_address, false);

  do
  {
    //! Displays the Read/Write menu
    Serial.print(F("  1-VID UP 4mV\n"));
    Serial.print(F("  2-VID DN 4mV\n"));
    Serial.print(F("  m-Main Menu\n"));
    Serial.print(F("\nEnter a command: "));

    user_command = read_int();                              //! Reads the user command
    if (user_command == 'm')                                // Print m if it is entered
    {
      Serial.print(F("m\n"));
    }
    else
      Serial.println(user_command);                         // Print user command

    switch (user_command)
    {
      case 1:
        vid_value += 0.004;
        pmbus->setVout (ltc2974_i2c_address, vid_value);
        Serial.print(F("LTC2974 VOUT "));
        Serial.println(vid_value, DEC);
        delay (1000);
        voltage = pmbus->readVout(ltc2974_i2c_address, false);
        Serial.print(F("LTC2974 VOUT Telemetry "));
        Serial.println(voltage, DEC);
        break;
      case 2:
        vid_value -= 0.004;
        pmbus->setVout (ltc2974_i2c_address, vid_value);
        Serial.print(F("LTC2974 VOUT "));
        Serial.println(vid_value, DEC);
        delay (1000);
        voltage = pmbus->readVout(ltc2974_i2c_address, false);
        Serial.print(F("LTC2974 VOUT Telemetry "));
        Serial.println(voltage, DEC);
        Serial.print(F("\n"));
        break;
      default:
        if (user_command != 'm')
          Serial.println(F("Invalid Selection"));
        break;
    }
  }
  while (user_command != 'm');
}

