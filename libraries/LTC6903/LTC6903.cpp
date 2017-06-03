/*!
LTC6903: 1kHz to 68MHz Serial Port Programmable Oscillator

@verbatim

The LTC6903/LTC6904 are low power self contained digital
frequency sources providing a precision frequency from
1kHz to 68MHz, set through a serial port. The LTC6903/
LTC6904 require no external components other than a
power supply bypass capacitor, and they operate over a
single wide supply range of 2.7V to 5.5V.

@endverbatim

http://www.linear.com/product/LTC6903

http://www.linear.com/product/LTC6903#demoboards

REVISION HISTORY
$Revision: 6237 $
$Date: 2016-12-20 15:09:16 -0800 (Tue, 20 Dec 2016) $

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

//! @ingroup RF_Timing
//! @{
//! @defgroup LTC6903 LTC6903: 1kHz to 68MHz Serial Port Programmable Oscillator
//! @}

/*! @file
    @ingroup LTC6903
    Library for LTC6903: 1kHz to 68MHz Serial Port Programmable Oscillator
*/

#include <stdint.h>
#include <Arduino.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "LTC6903.h"
#include <SPI.h>
#include "math.h"

// Writes 2 bytes
void LTC6903_write(uint8_t cs, uint16_t code)
{
  uint16_t rx;
  spi_transfer_word(cs, (uint16_t)code, &rx);
}

// Calculates the code necessary to create the clock frequency
// Returns the Code for the LTC6903
uint16_t LTC6903_frequency_to_code(float frequency, uint8_t clk)
{
  uint8_t oct;
  double oct_double;
  float float_dac;
  uint16_t dac;

  // Calculate OCT
  oct_double = log10((double)((frequency*1000000)/1039));
  oct_double *= 3.322;

  // Keep OCT within range
  if (oct_double>15)
    oct = 15;
  if (oct_double<0)
    oct = 0;
  oct = (uint8_t)oct_double;  // Cast as uint8_t , round down

  // Calculate DAC code
  float_dac = 2048-(2078*pow(2, (10+ oct)))/(frequency*1000000);   // Calculate the dac code
  float_dac = (float_dac > (floor(float_dac) + 0.5)) ? ceil(float_dac) : floor(float_dac);  // Round

  // Keep DAC within range
  if (float_dac>1023)
    float_dac = 1023;
  if (float_dac<0)
    float_dac = 0;

  dac = (uint16_t)float_dac;  // Cast as uint16_t

  return((uint32_t)((oct<<12)| (dac<<2) | clk));
}