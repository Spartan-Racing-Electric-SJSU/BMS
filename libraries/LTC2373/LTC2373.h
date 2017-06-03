/*!
LTC2373: 16/18-bit 1Msps 8 channel SAR ADC

@verbatim

The LTC2373_16/18 are pin-compatible, 16/18-bit A/D converters
with serial I/O, and an internal reference.

Example Code:

Read ADC input.

    adc_command = LTC2373_CH0 | LTC2373_UNIPOLAR_MODE | LTC2373_LOW_GAIN_MODE | LTC2373_NORMAL_MODE;     // Single-ended, CH0, unipolar, low gain, normal mode.
    LTC2373_read(LTC2373_CS, adc_command, &adc_code);   // Throws out last reading
    LTC2373_read(LTC2373_CS, adc_command, &adc_code);   // Obtains the current reading and stores to adc_code variable

    // Convert adc_code to voltage
    adc_voltage = LTC2373_code_to_voltage(adc_code, LTC2373_lsb, LTC2373_offset_unipolar_code);

@endverbatim

http://www.linear.com/product/LTC2373

http://www.linear.com/product/LTC2373#demoboards

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
    @ingroup LTC2373
    Header for LTC2373: 16/18-bit 1Msps 8 channel SAR ADC
*/

#ifndef LTC2373_H
#define LTC2373_H

#include <SPI.h>

//! Define the SPI CS pin
#ifndef LTC2373_CS
#define LTC2373_CS QUIKEVAL_CS
#endif

#define I2C_ADDRESS       0x20  //I2C address in 7 bit format
#define I2C_COMMAND       0x80  //Command to write to bus extender
#define LTC2373_SEQUENCER_BIT 0x80

//! @name LTC2373 Channel Addresses
//! @{
// Channel Address
#define LTC2373_CH0           0x40
#define LTC2373_CH1           0x48
#define LTC2373_CH2           0x50
#define LTC2373_CH3           0x58
#define LTC2373_CH4           0x60
#define LTC2373_CH5           0x68
#define LTC2373_CH6           0x70
#define LTC2373_CH7           0x78

#define LTC2373_CH0_1         0x00
#define LTC2373_CH1_0         0x20

#define LTC2373_CH2_3         0x08
#define LTC2373_CH3_2         0x28

#define LTC2373_CH4_5         0x10
#define LTC2373_CH5_4         0x30

#define LTC2373_CH6_7         0x18
#define LTC2373_CH7_6         0x38
//!@}

//! @name LTC2373 Uni/GAIN config bits
//! @{
// Range Command
#define LTC2373_RANGE_UNIPOLAR    0x00
#define LTC2373_RANGE_BIPOLAR   0x02
#define LTC2373_RANGE_DIFF_UNIPOLAR 0x04
#define LTC2373_RANGE_DIFF_BIPOLAR  0x06

// Gain Compression Command
#define LTC2373_NO_COMPRESSION   0x00
#define LTC2373_GAIN_COMPRESSION  0x01
//!@}

// Builds the ADC command
uint8_t LTC2373_build_command(uint8_t sequencer_bit,
                              uint8_t ch_designate,
                              uint8_t range_select,
                              uint8_t gain_compression
                             );


//! Reads the LTC2373 and returns 32-bit data
//! @return void
void LTC2373_read(uint8_t cs,           //!< Chip Select Pin
                  uint8_t adc_command,
                  uint32_t *ptr_adc_code    //!< Returns code read from ADC (from previous conversion)
                 );

//! Configures the LTC2373
//! @return void
void LTC2373_configure(uint8_t cs,
                       uint32_t adc_command
                      );

//! Calculates the LTC2373 input voltage given the binary data and lsb weight.
//! @return Floating point voltage
float LTC2373_code_to_voltage(uint8_t adc_command,
                              uint32_t adc_code,                   //!< Raw ADC code
                              float vref          //!< Reference voltage
                             );

#endif  //  LTC2373_H


