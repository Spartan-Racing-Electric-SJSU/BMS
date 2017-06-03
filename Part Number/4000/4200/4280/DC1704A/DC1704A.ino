/*!
Linear Technology DC874A Demonstration Board.
LTC4280: Hot Swap Controller with I2C Compatible Monitoring

@verbatim

NOTES
  Setup:
   Set the terminal baud rate to 115200 and select the newline terminator. Provide
   the board with an external power supply of 12 V.   Ensure all jumpers on the
   demo board are installed in their default positions from the factory. Refer to
   Demo Manual DC874A.

   This program has options to measure voltage at ADIN pin (input voltage), SOURCE
   VOLTAGE (output voltage), and SENSE CURRENT (current through sense resisitor).
   There are also options to read and manipulate CONTROL register, ALERT register,
   and FAULT register.

   Mass write option can be achieved using Device Address = 0xBE. Refer to datasheet
   LTC4280.

USER INPUT DATA FORMAT:
 decimal : 1024
 hex     : 0x400
 octal   : 02000  (leading 0 "zero")
 binary  : B10000000000
 float   : 1024.0

@endverbatim

http://www.linear.com/product/LTC4280

http://www.linear.com/product/LTC4280#demoboards

REVISION HISTORY
$Revision: 6066 $
$Date: 2016-11-17 09:23:26 -0800 (Thu, 17 Nov 2016) $

Copyright (c) 2013, Linear Technology Corp.(LTC)
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
    @ingroup LTC4280
*/

// Headerfiles
#include <Arduino.h>
#include <stdint.h>
#include "LT_I2C.h"
#include "LT_SPI.h"
#include "UserInterface.h"
#include "QuikEval_EEPROM.h"
#include <Wire.h>
#include <SPI.h>
#include "LTC4280.h"

// Macros
#define SENSE_RESISTOR 0.004

// Function Declarations
void print_title();
void print_prompt();
int8_t main_menu_1_continuous_mode();
int8_t main_menu_2_read_and_clear_faults();
int8_t main_menu_3_send_ARA();
int8_t main_menu_4_manage_alerts();
int8_t main_menu_5_settings();
int8_t main_menu_6_read_all_registers();

//! Initialize Linduino
void setup()
{
  quikeval_I2C_init();                    // Enable the I2C port
  quikeval_I2C_connect();
  Serial.begin(115200);                   // Initialize the serial port to the PC
  print_title();
  print_prompt();
}

//! Repeats Linduino loop
void loop()
{
  if (1)
  {
    int8_t ack = 0;                            // I2C acknowledge indicator
    static uint8_t user_command;               // The user input command
    if (Serial.available())                     //! Do nothing if serial is not available
    {
      user_command = read_int();                //! Read user input command
      if (user_command != 'm')
        Serial.println(user_command);
      Serial.println();
      ack = 0;
      switch (user_command)                     //! Prints the appropriate submenu
      {
        case 1:
          ack |= main_menu_1_continuous_mode();       // continous mode
          break;
        case 2:
          ack |= main_menu_2_read_and_clear_faults(); // read and clear faults
          break;
        case 3:
          ack |= main_menu_3_send_ARA();// read address of alert
          break;
        case 4:
          ack |= main_menu_4_manage_alerts();         // manage alerts
          break;
        case 5:
          ack |= main_menu_5_settings();              // settings
          break;
        case 6:
          ack |= main_menu_6_read_all_registers();
          break;
        default:
          Serial.println("Incorrect Option");
          break;
      }

      print_prompt();
    }
  }
}

//! Print the title block
void print_title()
{
  Serial.println(F("\n*****************************************************************"));
  Serial.print(F("* DC874A Demonstration Program                                  *\n"));
  Serial.print(F("*                                                               *\n"));
  Serial.print(F("* This program demonstrates how to send and receive data from   *\n"));
  Serial.print(F("* LTC4280 Hot Swap Controller with I2C Compatible Monitoring    *\n"));
  Serial.print(F("* Set the baud rate to 115200 and select the newline terminator.*\n"));
  Serial.print(F("*                                                               *\n"));
  Serial.print(F("*****************************************************************\n"));
}

//! Print the main menu
void print_prompt()
{
  Serial.print(F("\n\nMain Menu\n"));
  Serial.print(F("  1. Read current and voltage on continous mode\n"));
  Serial.print(F("  2. Read and clear faults\n"));
  Serial.print(F("  3. Request for alert response address (ARA)\n"));
  Serial.print(F("  4. Manage alerts\n"));
  Serial.print(F("  5. Settings\n"));
  Serial.print(F("  6. Read all registers\n\n"));
  Serial.print(F("Enter a command: "));
}

//! Function to read Rsense current, ADIN voltage and ADIN2 voltage in continous mode
int8_t main_menu_1_continuous_mode()
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
{
  int8_t ack = 0;
  float sense_voltage, source_voltage, adin_voltage;
  uint8_t sense_code, source_code, adin_code;
  float adin_lsb = (1.23/255), adin_resisive_ratio = (155.4/12.4);
  float source_lsb = (15.44/255), source_resisive_ratio = 1;  // Set voltage divider ratio to 1, for no resistor divider circuit.
  float sense_lsb = (0.03845/255), sense_resisive_ratio = 1;

  do
  {
    uint8_t faults;
    Serial.print(F("********** Press Enter to Exit ***********\n\n"));
    ack |= LTC4280_read(LTC4280_I2C_ADDRESS, LTC4280_FAULT_REG, &faults);
    if (faults != 0)
    {
      Serial.println(F("Faults Detected :"));
      if (faults & LTC4280_FET_SHORT_FAULT )
        Serial.println(F("  FET SHORT DETECTED"));
      if (faults & LTC4280_EN_STATE_CHANGE)
        Serial.println(F("  !EN STATE CHANGE DETECTED"));
      if (faults & LTC4280_POWER_BAD_FAULT )
        Serial.println(F("  BAD POWER DETECTED"));
      if (faults & LTC4280_OVERCURRENT_FAULT )
        Serial.println(F("  OVERCURRENT DETECTED"));
      if (faults & LTC4280_UNDERVOLTAGE_FAULT )
        Serial.println(F("  UNDERVOLTAGE DETECTED"));
      if (faults & LTC4280_OVERVOLTAGE_FAULT )
        Serial.println(F("  OVERVOLTAGE DETECTED"));
    }
    else
      Serial.println(F("  NO FAULTS DETECTED"));

    ack |= LTC4280_read(LTC4280_I2C_ADDRESS, LTC4280_SENSE_REG, &sense_code);
    ack |= LTC4280_read(LTC4280_I2C_ADDRESS, LTC4280_SOURCE_REG, &source_code);
    ack |= LTC4280_read(LTC4280_I2C_ADDRESS, LTC4280_ADIN_REG, &adin_code);

    sense_voltage = LTC4280_code_to_voltage(sense_code, sense_lsb, sense_resisive_ratio);
    source_voltage = LTC4280_code_to_voltage(source_code, source_lsb, source_resisive_ratio);
    adin_voltage = LTC4280_code_to_voltage(adin_code, adin_lsb, adin_resisive_ratio);

    Serial.print(F("\nSENSE VOLTAGE CODE: "));
    Serial.println(sense_code, HEX);
    Serial.print(F("SENSE CURRENT CALCULATED: "));
    Serial.println((sense_voltage / SENSE_RESISTOR), 4);
    Serial.print(F("\nSOURCE VOLTAGE CODE: "));
    Serial.println(source_code, HEX);
    Serial.print(F("SOURCE VOLTAGE CALCULATED: "));
    Serial.println(source_voltage, 4);
    Serial.print(F("\nADIN CODE: "));
    Serial.println(adin_code, HEX);
    Serial.print(F("VOLTAGE CALCULATED AT ADIN: "));
    Serial.println(adin_voltage, 4);
    Serial.print("\n");
    delay(2000);
  }
  while (Serial.available()==false && ack== 0);
  if (Serial.available())
    read_int(); // clears the Serial.available()
  return (ack);
}

//! Function to read and clear fault register
int8_t main_menu_2_read_and_clear_faults()
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
{
  int8_t ack = 0;

  uint8_t faults;
  ack |= LTC4280_read(LTC4280_I2C_ADDRESS, LTC4280_FAULT_REG, &faults);
  if (faults != 0)
  {
    Serial.println(F("Faults Detected :"));
    if (faults & LTC4280_FET_SHORT_FAULT)
      Serial.println(F("  FET SHORT DETECTED"));
    if (faults & LTC4280_EN_STATE_CHANGE)
      Serial.println(F("  !EN STATE CHANGE DETECTED"));
    if (faults & LTC4280_POWER_BAD_FAULT)
      Serial.println(F("  BAD POWER DETECTED"));
    if (faults & LTC4280_OVERCURRENT_FAULT)
      Serial.println(F("  OVERCURRENT DETECTED"));
    if (faults & LTC4280_UNDERVOLTAGE_FAULT)
      Serial.println(F("  UNDERVOLTAGE DETECTED"));
    if (faults & LTC4280_OVERVOLTAGE_FAULT)
      Serial.println(F("  OVERVOLTAGE DETECTED"));
  }
  ack |= LTC4280_write(LTC4280_I2C_ADDRESS, LTC4280_FAULT_REG, 0x00);
  Serial.print(F("\nALL FAULTS CLEARED \n\n"));

  return (ack);
}

//! Function to send alert response (0001100) and read back the address of device that pulled ALERT pin low.
int8_t main_menu_3_send_ARA()
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
{
  int8_t ack = 0;
  uint8_t address;
  ack |= LTC4280_ARA(LTC4280_I2C_ALERT_RESPONSE, &address);// send ARA
  Serial.print(F("  ALERT RESPONSE ADDRESS : 0x"));
  Serial.println(address,HEX);
  if (address == 0xFF && ack == 1)
  {
    ack = 0;
    Serial.print(F("  NO RESPONSE\n\n"));
  }
  return(ack);
}

//! Function to update alert register bits
int8_t main_menu_4_manage_alerts()
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
{
  int8_t ack = 0;
  uint8_t alert_settings = 0;
  int8_t user_command;
  do
  {
    Serial.println(F("GPIO OUTPUT"));
    Serial.println(F("  1. Enable"));
    Serial.println(F("  2. Disable"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
    if (user_command == 1)
      alert_settings |= LTC4280_GPIO_OUTPUT_ENABLE;
    else if (user_command ==2)
      alert_settings &= LTC4280_GPIO_OUTPUT_DISABLE;
    else if (user_command != 'm')
      Serial.println("Incorrect Option\n");
    else
      return(ack);

    Serial.println(F("FET SHORT ALERT"));
    Serial.println(F("  1. Enable"));
    Serial.println(F("  2. Disable"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
    if (user_command == 1)
      alert_settings |= LTC4280_FET_SHORT_ENABLE;
    else if (user_command ==2)
      alert_settings &= LTC4280_FET_SHORT_DISABLE;
    else if (user_command != 'm')
      Serial.println("Incorrect Option\n");
    else
      return(ack);

    Serial.println(F("!EN STATE CHANGE ALERT"));
    Serial.println(F("  1. Enable"));
    Serial.println(F("  2. Disable"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
    if (user_command == 1)
      alert_settings |= LTC4280_EN_STATE_ENABLE;
    else if (user_command ==2)
      alert_settings &= LTC4280_EN_STATE_DISABLE;
    else if (user_command != 'm')
      Serial.println("Incorrect Option\n");
    else
      return(ack);

    Serial.println(F("POWER BAD ALERT"));
    Serial.println(F("  1. Enable"));
    Serial.println(F("  2. Disable"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
    if (user_command == 1)
      alert_settings |= LTC4280_POWER_BAD_ENABLE;
    else if (user_command ==2)
      alert_settings &= LTC4280_POWER_BAD_DISABLE;
    else if (user_command != 'm')
      Serial.println("Incorrect Option\n");
    else
      return(ack);

    Serial.println(F("OVERCURRENT ALERT"));
    Serial.println(F("  1. Enable"));
    Serial.println(F("  2. Disable"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
    if (user_command == 1)
      alert_settings |= LTC4280_OVERCURRENT_ENABLE;
    else if (user_command ==2)
      alert_settings &= LTC4280_OVERCURRENT_DISABLE;
    else if (user_command != 'm')
      Serial.println("Incorrect Option\n");
    else
      return(ack);

    Serial.println(F("UNDERVOLTAGE ALERT"));
    Serial.println(F("  1. Enable"));
    Serial.println(F("  2. Disable"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
    if (user_command == 1)
      alert_settings |= LTC4280_UNDERVOLTAGE_ENABLE;
    else if (user_command ==2)
      alert_settings &= LTC4280_UNDERVOLTAGE_DISABLE;
    else if (user_command != 'm')
      Serial.println("Incorrect Option\n");
    else
      return(ack);

    Serial.println(F("OVERVOLTAGE ALERT"));
    Serial.println(F("  1. Enable"));
    Serial.println(F("  2. Disable"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
    if (user_command == 1)
      alert_settings |= LTC4280_OVERVOLTAGE_ENABLE;
    else if (user_command ==2)
      alert_settings &= LTC4280_OVERVOLTAGE_DISABLE;
    else if (user_command != 'm')
      Serial.println("Incorrect Option\n");
    else
      return(ack);
    ack |= LTC4280_write(LTC4280_I2C_ADDRESS, LTC4280_ALERT_REG, alert_settings);
    Serial.print(F("\n  ALERTS UPDATED\n\n"));
    Serial.println(F("  m. Main Menu"));
    Serial.println(F("  1. Repeat"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
  }
  while (!((user_command == 'm') || (ack)));
  return(ack);
}

//! Function to update control register bits
int8_t main_menu_5_settings()
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
{
  int8_t ack = 0;
  uint8_t settings =0;
  int8_t user_command;
  do
  {
    Serial.println(F("PGIO"));
    Serial.println(F("  1. !POWER GOOD"));
    Serial.println(F("  2. POWER GOOD"));
    Serial.println(F("  3. General Purpose Output"));
    Serial.println(F("  4. General Purpose Input"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
    {
      Serial.println("m");
      return(ack);
    }
    else
      Serial.println(user_command);
    Serial.println();
    switch (user_command)
    {
      case 1:
        settings |= LTC4280_PGIO_POWER_GOODX;
        break;
      case 2:
        settings |= LTC4280_PGIO_POWER_GOOD;
        break;
      case 3:
        settings |= LTC4280_PGIO_GENERAL_PURPOSE_OUTPUT;
        break;
      case 4:
        settings |= LTC4280_PGIO_GENERAL_PURPOSE_INPUT;
        break;
      default:
        if (user_command != 'm')
          Serial.println("Incorrect Option\n");
        break;
    }

    Serial.println(F("TEST MODE ENABLE"));
    Serial.println(F("  1. Enable"));
    Serial.println(F("  2. Disable"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
    if (user_command == 1)
      settings |= LTC4280_TEST_MODE_ENABLE;
    else if (user_command ==2)
      settings &= LTC4280_TEST_MODE_DISABLE;
    else if (user_command != 'm')
      Serial.println("Incorrect Option\n");
    else
      return(ack);

    Serial.println(F("MASS WRITE ENABLE"));
    Serial.println(F("  1. Enable"));
    Serial.println(F("  2. Disable"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
    if (user_command == 1)
      settings |= LTC4280_MASS_WRITE_ENABLE;
    else if (user_command ==2)
      settings &= LTC4280_MASS_WRITE_DISABLE;
    else if (user_command != 'm')
      Serial.println("Incorrect Option\n");
    else
      return(ack);

    Serial.println(F("FET STATUS"));
    Serial.println(F("  1. ON"));
    Serial.println(F("  2. OFF"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
    if (user_command == 1)
      settings |= LTC4280_FET_ON;
    else if (user_command ==2)
      settings &= LTC4280_FET_OFF;
    else if (user_command != 'm')
      Serial.println("Incorrect Option\n");
    else
      return(ack);

    Serial.println(F("OVERCURRENT AUTO RETRY"));
    Serial.println(F("  1. Enable"));
    Serial.println(F("  2. Disable"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
    if (user_command == 1)
      settings |= LTC4280_OVERCURRENT_AUTO_RETRY_ENABLE;
    else if (user_command ==2)
      settings &= LTC4280_OVERCURRENT_AUTO_RETRY_DISABLE;
    else if (user_command != 'm')
      Serial.println("Incorrect Option\n");
    else
      return(ack);

    Serial.println(F("UNDERVOLTAGE AUTO RETRY"));
    Serial.println(F("  1. Enable"));
    Serial.println(F("  2. Disable"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
    if (user_command == 1)
      settings |= LTC4280_UNDERVOLTAGE_AUTO_RETRY_ENABLE;
    else if (user_command ==2)
      settings &= LTC4280_UNDERVOLTAGE_AUTO_RETRY_DISABLE;
    else if (user_command != 'm')
      Serial.println("Incorrect Option\n");
    else
      return(ack);

    Serial.println(F("OVERVOLTAGE AUTO RETRY"));
    Serial.println(F("  1. Enable"));
    Serial.println(F("  2. Disable"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
    if (user_command == 1)
      settings |= LTC4280_OVERVOLTAGE_AUTO_RETRY_ENABLE;
    else if (user_command ==2)
      settings &= LTC4280_OVERVOLTAGE_AUTO_RETRY_DISABLE;
    else if (user_command != 'm')
      Serial.println("Incorrect Option\n");
    else
      return(ack);
    ack |= LTC4280_write(LTC4280_I2C_ADDRESS, LTC4280_CONTROL_REG, settings);
    Serial.print(F("\nSETTINGS UPDATED\n\n"));
    Serial.println(F("  m. Main Menu"));
    Serial.println(F("  1. Repeat"));
    Serial.print(F("Enter a command: "));
    user_command = read_int();
    if (user_command == 'm')
      Serial.println("m");
    else
      Serial.println(user_command);
    Serial.println();
  }
  while (!((user_command == 'm') || (ack)));
  return(ack);
}

//! Function to read all registers.
int8_t main_menu_6_read_all_registers()
//! @return Returns the state of the acknowledge bit after the I2C address read. 0=acknowledge, 1=no acknowledge.
{
  int8_t ack = 0;
  uint8_t faults;
  ack |= LTC4280_read(LTC4280_I2C_ADDRESS, LTC4280_FAULT_REG, &faults);
  Serial.print("  FAULT REGISTER   : 0b");
  Serial.println(faults, BIN);
  ack |= LTC4280_read(LTC4280_I2C_ADDRESS, LTC4280_STATUS_REG, &faults);
  Serial.print("  STATUS REGISTER  : 0b");
  Serial.println(faults, BIN);
  ack |= LTC4280_read(LTC4280_I2C_ADDRESS, LTC4280_ALERT_REG, &faults);
  Serial.print("  ALERT REGISTER   : 0b");
  Serial.println(faults, BIN);
  ack |= LTC4280_read(LTC4280_I2C_ADDRESS, LTC4280_CONTROL_REG, &faults);
  Serial.print("  CONTROL REGISTER : 0b");
  Serial.println(faults, BIN);
  Serial.println();
  return ack;
}