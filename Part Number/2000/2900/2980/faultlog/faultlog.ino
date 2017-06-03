/*!
Linear Technology LTC2980 Fault Logging
Note that LTC2980 = LTC2977 x 2

@verbatim

NOTES
  Setup:
   Set the terminal baud rate to 115200 and select the newline terminator.

@endverbatim

http://www.linear.com/product/LTC2980

REVISION HISTORY
$Revision: 4376 $
$Date: 2015-11-20 14:31:33 -0700 (Fri, 20 Nov 2015) $

Copyright (c) 2015, Linear Technology Corp.(LTC)
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
    @ingroup LTC2980
*/

#include <Arduino.h>
#include <stdint.h>
#include <avr/pgmspace.h>
#include "Linduino.h"
#include "UserInterface.h"
#include "LT_Wire.h"
#include "LT_SMBusNoPec.h"
#include "LT_SMBusPec.h"
#include "LT_PMBus.h"
#include "LT_2977FaultLog.h"

#define FILE_TEXT_LINE_MAX 132

#define LTC2977_I2C_ADDRESS_A 0x30
#define LTC2977_I2C_ADDRESS_B 0x32

// Global variables
static uint8_t ltc2977_i2c_address_a;
static uint8_t ltc2977_i2c_address_b;

static LT_SMBus *smbus = new LT_SMBusPec();
static LT_PMBus *pmbus = new LT_PMBus(smbus);

static LT_2977FaultLog *faultLog2977_a = new LT_2977FaultLog(pmbus);
static LT_2977FaultLog *faultLog2977_b = new LT_2977FaultLog(pmbus);

//! Initialize Linduino
//! @return void
void setup()
{
  Serial.begin(115200);         //! Initialize the serial port to the PC
  print_title();
  ltc2977_i2c_address_a = LTC2977_I2C_ADDRESS_A;
  ltc2977_i2c_address_b = LTC2977_I2C_ADDRESS_B;
  print_prompt();
}

//! Repeats Linduino loop
//! @return void
void loop()
{
  uint8_t user_command;
  uint8_t *addresses = NULL;

  struct LT_2977FaultLog::FaultLogLtc2977 *ltc2977_fault_log_a;
  struct LT_2977FaultLog::FaultLogLtc2977 *ltc2977_fault_log_b;


  if (Serial.available())                          //! Checks for user input
  {
    while ((user_command = read_int()) == 0);     //! Reads the user command
    if (user_command != 'm')
      Serial.println(user_command);

    uint8_t status_a;
    uint8_t status_b;

    switch (user_command)                          //! Prints the appropriate submenu
    {
      case 1:
        status_a = pmbus->readMfrFaultLogStatusByte(ltc2977_i2c_address_a);
        status_b = pmbus->readMfrFaultLogStatusByte(ltc2977_i2c_address_b);

        if (status_a & LTC2977_SFL_EEPROM)  // A is one of the two LTC2977's inside of the LTC2980
        {
          Serial.println(F("LTC2980 A Fault Log"));
          faultLog2977_a->read(ltc2977_i2c_address_a);

          faultLog2977_a->dumpBinary(&Serial);
          Serial.println();
          faultLog2977_a->print(&Serial);

          faultLog2977_a->release();
        }
        else
          Serial.println(F("No LTC2980 A Fault Log"));

        if (status_b & LTC2977_SFL_EEPROM)  // B is one of the two LTC2977's inside of the LTC2980
        {
          Serial.println(F("LTC2980 B Fault Log"));
          faultLog2977_b->read(ltc2977_i2c_address_b);

          faultLog2977_b->dumpBinary(&Serial);
          Serial.println();
          faultLog2977_b->print(&Serial);

          faultLog2977_b->release();
        }
        else
          Serial.println(F("No LTC2980 B Fault Log"));

        break;
      case 2:
        faultLog2977_a->clearFaultLog(ltc2977_i2c_address_a);
        faultLog2977_b->clearFaultLog(ltc2977_i2c_address_b);

        break;
      case 3:
        pmbus->clearAllFaults(ltc2977_i2c_address_a);
        pmbus->clearAllFaults(ltc2977_i2c_address_b);

        break;
      case 4:
        pmbus->enablePec(ltc2977_i2c_address_a);
        pmbus->enablePec(ltc2977_i2c_address_b);
        delete faultLog2977_a;
        delete faultLog2977_b;
        delete smbus;
        delete pmbus;
        smbus = new LT_SMBusPec();
        pmbus = new LT_PMBus(smbus);
        faultLog2977_a = new LT_2977FaultLog(pmbus);
        faultLog2977_b = new LT_2977FaultLog(pmbus);
        break;
      case 5:
        pmbus->disablePec(ltc2977_i2c_address_a);
        pmbus->disablePec(ltc2977_i2c_address_b);
        delete faultLog2977_a;
        delete faultLog2977_b;
        delete smbus;
        delete pmbus;
        smbus = new LT_SMBusNoPec();
        pmbus = new LT_PMBus(smbus);
        faultLog2977_a = new LT_2977FaultLog(pmbus);
        faultLog2977_b = new LT_2977FaultLog(pmbus);
        break;
      case 6:
        addresses = smbus->probe(0);
        while (*addresses != 0)
        {
          Serial.print(F("ADDR 0x"));
          Serial.println(*addresses++, HEX);
        }
        break;
      case 7:
        pmbus->restoreFromNvmGlobal();
        delay(2000);
        pmbus->resetGlobal();
        break;
      case 8:
        smbus->sendByte(ltc2977_i2c_address_a, MFR_FAULT_LOG_STORE);
        smbus->sendByte(ltc2977_i2c_address_b, MFR_FAULT_LOG_STORE);
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
  Serial.print(F("\n***************************************************************\n"));
  Serial.print(F("* LTC2980 Fault Log Program                                     *\n"));
  Serial.print(F("*                                                               *\n"));
  Serial.print(F("* This program dumps fault logs                                 *\n"));
  Serial.print(F("*                                                               *\n"));
  Serial.print(F("* Set the baud rate to 115200 and select the newline terminator.*\n"));
  Serial.print(F("*                                                               *\n"));
  Serial.print(F("*****************************************************************\n"));
}

//! Prints main menu.
//! @return void
void print_prompt()
{
  Serial.print(F("\n  1-Dump Fault Logs\n"));
  Serial.print(F("  2-Clear Fault Logs\n"));
  Serial.print(F("  3-Clear Faults\n"));
  Serial.print(F("  4-PEC On\n"));
  Serial.print(F("  5-PEC Off\n"));
  Serial.print(F("  6-Bus Probe\n"));
  Serial.print(F("  7-Reset\n"));
  Serial.print(F("  8-Store Fault Log\n"));
  Serial.print(F("\nEnter a command:"));
}


