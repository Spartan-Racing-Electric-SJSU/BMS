/*!
Linear Technology DC1703A Demonstration Board.
LTC2655: Quad I2C 16-/12-Bit Rail-to-Rail DACs with 10ppm/C Max Reference.

@verbatim
NOTES
 Setup:
   Set the terminal baud rate to 115200 and select the newline terminator.
   The program displays calculated voltages which are based on the voltage
   of the reference used, be it internal or external. A precision voltmeter
   is needed to verify the actual measured voltages against the calculated
   voltage displayed. If an external reference is used, a precision voltage
   source is required to apply the external reference voltage. A precision
   voltmeter is also required to measure the external reference voltage.
   No external power supply is required. Any assembly option may be used:
   DC1703A-A, DC1703A-B, DC1703A-C, DC1703A-D.


 Explanation of Commands:
   1- Select DAC: Select one of four DACs to test: A, B, C, D.

   2- Write to DAC input register: Value is stored in the DAC for updating
      later, allowing multiple channels to be updated at once, either
      through a software "Update All" command or by asserting the LDAC# pin.
      User will be prompted to enter either a code in hex or decimal, or a
      voltage. If a voltage is entered, a code will be calculated based on
      the active scaling and reference parameters - ideal values if no
      calibration was ever stored.

   3- Write and Update: Similar to item 1, but DAC is updated immediately.

   4- Update DAC: Copies the value from the input register into the DAC
      Register. Note that a "write and update" command writes the code to
      BOTH the input register and DAC register, so subsequent "update" commands
      will simply re-copy the same data (no change in output).

   5- Power Down DAC: Disable DAC output. Power supply current is reduced. DAC
      code present in DAC registers at time of shutdown are preserved.

   6- Set reference mode, either internal or external: Selecting external mode
      prompts for the external refernce voltage, which is used directly if no
      individual DAC calibration is stored. The selection and entered volgage
      are stored to EEPROM so it is persisent across reset / power cycles.

   7- Calibrate DAC: Use a precision voltmeter to obtain and enter VOUT readings
      taken with different DAC codes. Set reference mode FIRST, as values are stored
      separately for internal and external reference mode. Entries are used to
      calculate the closest code to send to the DAC to achieve an entered voltage.

   8- Enable / Disable calibration: Switch between stored calibration values and
      defaults. Calibration parameters are stored separately for internal and
      external reference modes. Ideal calibration will be used if the calibration
      parameter valid key is not set.


USER INPUT DATA FORMAT:
 decimal : 1024
 hex     : 0x400
 octal   : 02000  (leading 0 "zero")
 binary  : B10000000000
 float   : 1024.0

@endverbatim

http://www.linear.com/product/LTC2655

http://www.linear.com/product/LTC2655#demoboards

REVISION HISTORY
$Revision: 3659 $
$Date: 2015-07-01 10:19:20 -0700 (Wed, 01 Jul 2015) $

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
    @ingroup LTC2655
*/

#include <Arduino.h>
#include <stdint.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "UserInterface.h"
#include "LT_I2C.h"
#include "QuikEval_EEPROM.h"
#include "LTC2655.h"
#include <Wire.h>
#include <SPI.h>

//! Calibration associated with internal reference.
//! This value is stored in EEPROM_CAL address of QuikEval EEPROM to indicate that a calibration occurred using the internal reference.
#define EEPROM_CAL_KEY_INT  0x5678
//! Calibration associated with external reference.
//! This value is stored in EEPROM_CAL address of QuikEval EEPROM to indicate that a calibration occurred using the external reference.
#define EEPROM_CAL_KEY_EXT  0x9ABC          //!< Calibration associated with external reference. (Stored in EEPROM_CAL address of QuikEval EEPROM).

// DAC Reference State
// Could have been 0 or 1. This allows you to use the
// variable "reference_mode" as the command argument to a write
#define REF_INTERNAL LTC2655_CMD_INTERNAL_REFERENCE   //!< Stored reference state is Internal
#define REF_EXTERNAL LTC2655_CMD_EXTERNAL_REFERENCE   //!< Stored reference state is External

// EEPROM memory locations
#define STORED_REF_STATE_BASE     EEPROM_CAL_STATUS_ADDRESS     //!< Base address of the stored reference state
#define INT_CAL_VALID_BASE        STORED_REF_STATE_BASE + 2     //!< Base address of the "internal ref calibration valid" flag
#define INT_CAL_PARAMS_BASE       INT_CAL_VALID_BASE + 2        //!< Base address of the internal ref calibration parameters
#define EXT_CAL_VALID_BASE        INT_CAL_PARAMS_BASE + 32      //!< Base address of the "external ref calibration valid" flag
#define EXT_CAL_PARAMS_BASE       EXT_CAL_VALID_BASE + 2        //!< Base address of the external ref calibration parameters
#define EXT_REF_V_BASE            EXT_CAL_PARAMS_BASE + 32      //!< Base address of the stored external reference voltage

// Function Declaration
int8_t restore_calibration();               // Read the DAC calibration from EEPROM, Return 1 if successful, 0 if not
void store_calibration();                   // Store the DAC calibration to the EEPROM
void print_title();                         // Print the title block
void print_prompt(int16_t selected_dac);    // Prompt the user for an input command
int16_t prompt_voltage_or_code();
uint16_t get_voltage(float LTC2655_lsb, int16_t LTC2655_offset);
uint16_t get_code();
int8_t calibrate_dac(uint8_t index);        // Calibrate the selected DAC using a voltmeter. The routine does a linear curve fit given two data points.

int8_t menu_1_select_dac(int16_t *selected_dac);
int8_t menu_2_write_to_input_register(int16_t selected_dac);
int8_t menu_3_write_and_update_dac(int16_t selected_dac);
int8_t menu_4_update_power_up_dac(int16_t selected_dac);
int8_t menu_5_power_down_dac(int16_t selected_dac);
int8_t menu_6_set_reference_mode();         // int, ext, if ext, prompt for voltage
int8_t menu_7_calibrate_dacs();
int8_t menu_8_enable_calibration();

// Global variables
static uint8_t demo_board_connected;               //!< Set to 1 if the board is connected
static uint8_t shift_count = 0;                    //!< The data align shift count. For 16-bit=0, for 12-bits=4
static uint8_t reference_mode;                     //!< Tells whether to set internal or external reference

// Global calibration variables
static float reference_voltage;                    //!< Reference voltage, either internal or external
static int16_t LTC2655_offset[5];                  //!< DAC offset - index 4 for "all DACs." If part is calibrated, then index 0 is stored to "all DACs."
static float LTC2655_lsb[5];                       //!< The LTC2655 lsb - index 4 for "all DACs." If part is calibrated, then index 0 is stored to "all DACs."

// Constants

//! Lookup table for DAC address. Allows the "All DACs" address to be indexed right after DAC D in loops.
//! This technique is very useful for devices with non-monotonic channel addresses.
const uint8_t address_map[5] = {LTC2655_DAC_A, LTC2655_DAC_B, LTC2655_DAC_C, LTC2655_DAC_D, LTC2655_DAC_ALL};  //!< Map entered option 0..2 to DAC address

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
  char demo_name[] = "DC1703";      // Demo Board Name stored in QuikEval EEPROM
  quikeval_I2C_init();              // Configure the EEPROM I2C port for 100khz
  quikeval_I2C_connect();           // Connect I2C to main data port
  Serial.begin(115200);             // Initialize the serial port to the PC
  print_title();
  demo_board_connected = discover_demo_board(demo_name);
  if (demo_board_connected)
  {
    restore_calibration();
    print_prompt(0);
  }
}

//! Repeats Linduino loop
void loop()
{
  int8_t ack=0;
  int16_t user_command;
  static  int16_t selected_dac = 0;     // The selected DAC to be updated (0=A, 1=B ... 5=All).  Initialized to "A".
  // The main control loop
  if (demo_board_connected)             // Do nothing if the demo board is not connected
  {
    if (Serial.available())             // Check for user input
    {
      user_command = read_int();        // Read the user command
      Serial.println(user_command);
      Serial.flush();
      ack = 0;
      switch (user_command)
      {
        case 1:
          ack |= menu_1_select_dac(&selected_dac);
          break;
        case 2:
          ack |= menu_2_write_to_input_register(selected_dac);
          break;
        case 3:
          ack |= menu_3_write_and_update_dac(selected_dac);
          break;
        case 4:
          ack |= menu_4_update_power_up_dac(selected_dac);
          break;
        case 5:
          ack |= menu_5_power_down_dac(selected_dac);
          break;
        case 6:
          ack |= menu_6_set_reference_mode(); // int, ext, if ext, prompt for voltage
          ack |= restore_calibration();
          break;
        case 7:
          ack |= menu_7_calibrate_dacs();
          restore_calibration();
          break;
        case 8:
          menu_8_enable_calibration();
          restore_calibration();
          break;
        default:
          Serial.println("Incorrect Option");
          break;
      }
      if (ack) Serial.println("I2C NACK received, check address\n");
      Serial.println("\n*****************************************************************");
      print_prompt(selected_dac);
    }
  }
}

// Function Definitions

//! Select which DAC to operate on
//! @return 0
int8_t menu_1_select_dac(int16_t *selected_dac)
{
  // Select a DAC to operate on
  Serial.print("Select DAC to operate on (0=A, 1=B, 2=C, 3=D, 4=All)");
  *selected_dac = read_int();
  if (*selected_dac == 4)
    Serial.println("All");
  else
    Serial.println(*selected_dac);
  return(0);
}

//! Write data to input register, but do not update DAC output
//! @return ACK bit (0=acknowledge, 1=no acknowledge)
int8_t menu_2_write_to_input_register(int16_t selected_dac)
{
  int8_t ack=0;
  uint16_t dac_code;

  if (prompt_voltage_or_code() == PROMPT_VOLTAGE)
    dac_code = get_voltage(LTC2655_lsb[selected_dac], LTC2655_offset[selected_dac]);
  else
    dac_code = get_code();

  ack |= LTC2655_write(LTC2655_I2C_ADDRESS, LTC2655_CMD_WRITE, address_map[selected_dac], dac_code << shift_count);
  return (ack);
}

//!Write data to DAC register (which updates output immediately)
//! @return ACK bit (0=acknowledge, 1=no acknowledge)
int8_t menu_3_write_and_update_dac(int16_t selected_dac)
{
  int8_t ack=0;
  uint16_t dac_code;

  if (prompt_voltage_or_code() == PROMPT_VOLTAGE)
    dac_code = get_voltage(LTC2655_lsb[selected_dac], LTC2655_offset[selected_dac]);
  else
    dac_code = get_code();

  ack |= LTC2655_write(LTC2655_I2C_ADDRESS, LTC2655_CMD_WRITE_UPDATE, address_map[selected_dac], dac_code << shift_count);
  return (ack);
}

//! Update DAC with data that is stored in input register, power up if sleeping
//! @return ACK bit (0=acknowledge, 1=no acknowledge)
int8_t menu_4_update_power_up_dac(int16_t selected_dac)
{
  // Update DAC
  int8_t ack=0;
  ack |= LTC2655_write(LTC2655_I2C_ADDRESS, LTC2655_CMD_UPDATE, address_map[selected_dac], 0x0000);
  return (ack);
}

//! Power down DAC
//! @return ACK bit (0=acknowledge, 1=no acknowledge)
int8_t menu_5_power_down_dac(int16_t selected_dac)
{
  // Power down DAC
  int8_t ack=0;
  ack |= LTC2655_write(LTC2655_I2C_ADDRESS, LTC2655_CMD_POWER_DOWN, address_map[selected_dac], 0x0000);
  return (ack);
}

//! Set reference mode and store to EEPROM
//! @return 0
int8_t menu_6_set_reference_mode(void) // int, ext, if ext, prompt for voltage
{
  int16_t user_input;
  Serial.println("Select reference mode - 0 for Internal, 1 for External");
  user_input = read_int();
  if (user_input == 1)
  {
    reference_mode = REF_EXTERNAL;
    Serial.println("External reference mode; enter external reference voltage");
    reference_voltage = read_float();
    Serial.print(reference_voltage, 5);
    Serial.println("V");
    eeprom_write_float(EEPROM_I2C_ADDRESS, reference_voltage, EXT_REF_V_BASE);
  }
  else
  {
    reference_mode = REF_INTERNAL;
    Serial.println("Internal reference mode selected");
  }
  Serial.println("Writing reference mode to EEPROM\n\n");
  eeprom_write_byte(EEPROM_I2C_ADDRESS, reference_mode, STORED_REF_STATE_BASE);
  return(0);
}

//! Calibrate all DACs by measuring two known outputs
//! @return 0
int8_t menu_7_calibrate_dacs()
{
  // Calibrate the DACs using a multi-meter
  uint8_t i;
  for (i = 0; i < 4; i++)
  {
    calibrate_dac(i);   // Run calibration routine
  }
  store_calibration();
  return (0);
}

//! Enable / Disable calibration. Use with caution - behavior is undefined if you enable calibration and an actual
//! calibration cycle has not been performed.
//! @return 0
int8_t menu_8_enable_calibration()
{
  int16_t user_input;
  Serial.println(F("\n\nSelect option -  0: Enable Internal, 1: Disable Internal, 2: Enable External, 3: Disable External"));
  user_input = read_int();
  switch (user_input)
  {
    case 0:
      Serial.println(F("Enabling Internal Cal Params"));
      eeprom_write_int16(EEPROM_I2C_ADDRESS, EEPROM_CAL_KEY, INT_CAL_VALID_BASE);
      break;
    case 1:
      Serial.println(F("Disabling Internal Cal Params"));
      eeprom_write_int16(EEPROM_I2C_ADDRESS, 0x0000, INT_CAL_VALID_BASE);
      break;
    case 2:
      Serial.println(F("Enabling External Cal Params"));
      eeprom_write_int16(EEPROM_I2C_ADDRESS, EEPROM_CAL_KEY, EXT_CAL_VALID_BASE);
      break;
    case 3:
      Serial.println(F("Disabling External Cal Params"));
      eeprom_write_int16(EEPROM_I2C_ADDRESS, 0x0000, EXT_CAL_VALID_BASE);
      break;
  }
  return(0);
}

//! Read stored calibration parameters from nonvolatile EEPROM on demo board
//! @return Return 1 if successful, 0 if not
int8_t restore_calibration()
// Read the DAC calibration from EEPROM
// Return 1 if successful, 0 if not
{
  int8_t ack=0;
  int16_t intvalid, extvalid;
  uint8_t i, eeaddr;
  float dac_count;                                  // The number of codes, 4096 for 12 bits, 65536 for 16 bits

  Serial.println(F("\n\nReading Calibration parameters from EEPROM..."));
  float full_scale; // To avoid confusion - in internal ref mode, FS=Vref, in ext mode, FS=2*Vref
  // Read the cal keys from the EEPROM.
  eeprom_read_int16(EEPROM_I2C_ADDRESS, &intvalid, INT_CAL_VALID_BASE);
  eeprom_read_int16(EEPROM_I2C_ADDRESS, &extvalid, EXT_CAL_VALID_BASE);
  // Read the stored reference state
  eeprom_read_byte(EEPROM_I2C_ADDRESS, (char *) &reference_mode, STORED_REF_STATE_BASE);
  // Read external ref V unconditionally, overwrite with defaults if no cal found
  eeprom_read_float(EEPROM_I2C_ADDRESS, &reference_voltage, EXT_REF_V_BASE);

  if (reference_mode == REF_EXTERNAL)
  {
    Serial.println(F("Restored external ref. Voltage:"));
    Serial.println(reference_voltage, 5);
  }
  else  // EITHER reference is set to internal, OR not programmed in which case default to internal
  {
    reference_mode = REF_INTERNAL; // Redundant if already set
    Serial.println("Internal reference mode set");
  }

  // Write the reference mode to the DAC right away
  ack |= LTC2655_write(LTC2655_I2C_ADDRESS, reference_mode, 0x0F, 0x0000);

  // Set up default values, shift count, DAC count
  // Calibration parameters MAY be changed next, if match
  // between reference mode and stored calibration
  full_scale = reference_voltage * 2.0; // If external ref mode, this applies.
  switch (demo_board.option)
  {
    case 'A':
      // LTC2655CUF-L16, 16-bits, 2.5V full scale
      shift_count = 0;
      if (reference_mode == REF_INTERNAL) full_scale = 2.5;
      dac_count = 65536;
      break;
    case 'B':
      // LTC2655CUF-H16, 16-bits, 4.096V full scale
      shift_count = 0;
      if (reference_mode == REF_INTERNAL) full_scale = 4.096;
      dac_count = 65536;
      break;
    case 'C':
      // LTC2655CUF-L12, 12-bits, 2.5V full scale
      shift_count = 4;
      if (reference_mode == REF_INTERNAL) full_scale = 2.5;
      dac_count = 4096;
      break;
    case 'D':
      // LTC2655CUF-H12, 12-bits, 4.096V full scale
      shift_count = 4;
      if (reference_mode == REF_INTERNAL) full_scale = 4.096;
      dac_count = 4096;
      break;
  }

  for (i = 0; i <= 4; i++)
  {
    LTC2655_offset[i] = 0;
    LTC2655_lsb[i] = full_scale / dac_count;
  }

  // Restore calibration IF reference mode matches stored calibraiton
  eeaddr = 0;   // Assume no calibration present or mismatch between cal and reference mode

  if ((intvalid == EEPROM_CAL_KEY) && (reference_mode == REF_INTERNAL))
  {
    eeaddr = INT_CAL_PARAMS_BASE;
    Serial.println(F("Found internal calibration, restoring...)"));
  }
  else if ((extvalid == EEPROM_CAL_KEY) && (reference_mode == REF_EXTERNAL))
  {
    eeaddr = EXT_CAL_PARAMS_BASE;
    Serial.println(F("Found external calibration, restoring...)"));
  }
  else Serial.println(F("Calibration not found for this\nreference mode, using ideal calibration"));

  if (eeaddr != 0) // If cal key was enabled and reference mode is correct, read offset and lsb
  {
    eeprom_read_int16(EEPROM_I2C_ADDRESS, &LTC2655_offset[0], eeaddr);
    eeprom_read_int16(EEPROM_I2C_ADDRESS, &LTC2655_offset[1], eeaddr + 2);
    eeprom_read_int16(EEPROM_I2C_ADDRESS, &LTC2655_offset[2], eeaddr + 4);
    eeprom_read_int16(EEPROM_I2C_ADDRESS, &LTC2655_offset[3], eeaddr + 6);
    eeprom_read_float(EEPROM_I2C_ADDRESS, &LTC2655_lsb[0], eeaddr + 8);
    eeprom_read_float(EEPROM_I2C_ADDRESS, &LTC2655_lsb[1], eeaddr + 12);
    eeprom_read_float(EEPROM_I2C_ADDRESS, &LTC2655_lsb[2], eeaddr + 16);
    eeprom_read_float(EEPROM_I2C_ADDRESS, &LTC2655_lsb[3], eeaddr + 20);
    LTC2655_offset[4] = LTC2655_offset[0]; // Copy cal value for DAC A to cal value for
    LTC2655_lsb[4] = LTC2655_lsb[0];       // DAC ALL
    Serial.println("Calibration Restored");
  }
  for (i=0; i<=4; ++i)
  {
    Serial.print("DAC ");
    Serial.print((char) ('A' + i));
    Serial.print(" offset: ");
    Serial.print(LTC2655_offset[i]);
    Serial.print(" , lsb: ");
    Serial.print(LTC2655_lsb[i]*1000, 4);
    Serial.println(" mv");
  }
  Serial.println("(DAC E applies to ALL DACs selections)");
  if (eeaddr != 0) return (1);
  return (ack);
}

//! Store measured calibration parameters to nonvolatile EEPROM on demo board
void store_calibration()
// Store the DAC calibration to the EEPROM
{
  uint8_t eeaddr;
  if (reference_mode == REF_INTERNAL)
  {
    eeprom_write_int16(EEPROM_I2C_ADDRESS, EEPROM_CAL_KEY, INT_CAL_VALID_BASE);
    eeaddr = INT_CAL_PARAMS_BASE;
  }
  else
  {
    eeprom_write_int16(EEPROM_I2C_ADDRESS, EEPROM_CAL_KEY, EXT_CAL_VALID_BASE);
    eeaddr = EXT_CAL_PARAMS_BASE;
  }

  eeprom_write_int16(EEPROM_I2C_ADDRESS, LTC2655_offset[0], eeaddr);        // Offset
  eeprom_write_int16(EEPROM_I2C_ADDRESS, LTC2655_offset[1], eeaddr + 2);
  eeprom_write_int16(EEPROM_I2C_ADDRESS, LTC2655_offset[2], eeaddr + 4);
  eeprom_write_int16(EEPROM_I2C_ADDRESS, LTC2655_offset[3], eeaddr + 6);
  eeprom_write_float(EEPROM_I2C_ADDRESS, LTC2655_lsb[0], eeaddr + 8);      // lsb
  eeprom_write_float(EEPROM_I2C_ADDRESS, LTC2655_lsb[1], eeaddr + 12);
  eeprom_write_float(EEPROM_I2C_ADDRESS, LTC2655_lsb[2], eeaddr + 16);
  eeprom_write_float(EEPROM_I2C_ADDRESS, LTC2655_lsb[3], eeaddr + 20);
  Serial.println(F("Calibration Stored to EEPROM"));
}

//! Prompt user to enter a voltage or digital code to send to DAC
//! @return prompt type
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
//! @return voltage
uint16_t get_voltage(float LTC2655_lsb, int16_t LTC2655_offset)
{
  float dac_voltage;

  Serial.print(F("Enter Desired DAC output voltage: "));
  dac_voltage = read_float();
  Serial.print(dac_voltage);
  Serial.println(" V");
  Serial.flush();
  return(LTC2655_voltage_to_code(dac_voltage, LTC2655_lsb, LTC2655_offset));
}

//! Get code to send to DAC directly, in decimal, hex, or binary
//! @return DAC code
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
  Serial.println(F("* DC1703 Demonstration Program                                  *"));
  Serial.println(F("*                                                               *"));
  Serial.println(F("* This program demonstrates how to send data to the LTC2655     *"));
  Serial.println(F("* quad 16/12-bit DAC found on the DC1703 demo board.            *"));
  Serial.println(F("*                                                               *"));
  Serial.println(F("* Set the baud rate to 115200 and select the newline terminator.*"));
  Serial.println(F("*                                                               *"));
  Serial.println(F("*****************************************************************"));
}

//! Prints main menu.
void print_prompt(int16_t selected_dac)
{
  Serial.println(F("\nCommand Summary:"));
  Serial.println(F("  1-Select DAC"));
  Serial.println(F("  2-Write to input register (no update)"));
  Serial.println(F("  3-Write and update DAC"));
  Serial.println(F("  4-Update / power up DAC"));
  Serial.println(F("  5-Power down DAC"));
  Serial.println(F("  6-Set reference mode"));
  Serial.println(F("  7-Calibrate DAC"));
  Serial.println(F("  8-Enable / Disable calibration"));

  Serial.println("\nPresent Values:");
  Serial.print("  Selected DAC: ");
  if (selected_dac != 4)
    Serial.println((char) (selected_dac + 0x41));
  else
    Serial.println("All");
  //Serial.print("  DAC Code: 0x");
  // Serial.println(code[selected_dac], HEX);
  // Serial.print("  Calculated DAC Output Voltage = ");
  // Serial.print(voltage, 4);
  // Serial.println(" V");
  Serial.print("  DAC Reference: ");
  if (reference_mode == REF_INTERNAL)
    Serial.println("Internal");
  else
  {
    Serial.print(F("External "));
    Serial.print(reference_voltage, 5);
    Serial.println(F("V reference, please verify"));
    Serial.print(F("Enter a command:"));

  }
  Serial.flush();
}

//! Calibrate the selected DAC using a voltmeter. The routine
//! does a linear curve fit given two data points.
//! @return ACK bit (0=acknowledge, 1=no acknowledge)
int8_t calibrate_dac(uint8_t index)
{
  int8_t ack=0;
  uint16_t code1 = 0x0200;                            //! Calibration code 1
  uint16_t code2 = 0xFFFF;                            //! Calibration code 2
  float voltage1;                                     //! Calibration voltage 1
  float voltage2;                                     //! Calibration voltage 2
  Serial.println("");
  Serial.print("Calibrating DAC ");
  Serial.println((char) (0x41 + index));
  // Left align 12-bit code1 to 16 bits & write to DAC
  ack |= LTC2655_write(LTC2655_I2C_ADDRESS, LTC2655_CMD_WRITE_UPDATE, index, code1 << shift_count);
  Serial.print("DAC code set to 0x");
  Serial.println(code1, HEX);
  Serial.print("Enter measured DAC voltage:");
  voltage1 = read_float();
  Serial.print(voltage1, 6);
  Serial.println(" V");
  // Left align 12-bit code2 to 16 bits & write to DAC
  ack |= LTC2655_write(LTC2655_I2C_ADDRESS, LTC2655_CMD_WRITE_UPDATE, index, code2 << shift_count);
  Serial.print("DAC code set to 0x");
  Serial.println(code2, HEX);
  Serial.print("Enter measured DAC voltage:");
  voltage2 = read_float();
  Serial.print(voltage2, 6);
  Serial.println(" V");
  LTC2655_calibrate(code1, code2, voltage1, voltage2, &LTC2655_lsb[index], &LTC2655_offset[index]);
  return(ack);
}
