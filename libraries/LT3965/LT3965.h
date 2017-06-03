/*!
LT3965 - Octal Matrix LED Bypass Switch

@verbatim

The LT3965 is an LED bypass switching device for dimming indiviadual LEDs in a
string using a common current source. It features eight individually controlled
floating source N-channel MOSFET switches rated for 17V/330 mΩ. The eight
switches can be connected in parallel and/or in series to bypass current around
one or more LEDs in a string. The LT3965 as a slave uses the I2C serial interface
to communicate with the master. Each of eight channels can be independently
programmed to bypass the LED string in constant on, constant off, dimming without
fade transition or dimming with fade transition mode.

WRITE PROTOCOLS (MSB FIRST):

            Byte #1                     Byte #2                         Byte #3

ACMODE WRITE      0   1   0   A4  A3  A2  A1  0(W)        B7  B6  B5  B4  B3  B2  B1  B0
SCMODE WRITE SHORT    1   0   1   A4  A3  A2  A1  0(W)    0   C3  C2  C1  B3  B2  B1  B0
SCMODE WRITE LONG   1   0   1   A4  A3  A2  A1  0(W)    1   C3  C2  C1  B11 B10 B9  B8    B7  B6  B5  B4  B3  B2  B1  B0
BCMODE WRITE      0   0   0   1   1   0   0   0(W)

W           : I2C Write (0)
R           : I2C Read  (1)
Ax          : Device Address
Bx          : Data Bits
Cx          : Channel Address


@endverbatim


http://www.linear.com/product/LT3965

http://www.linear.com/product/LT3965#demoboards

REVISION HISTORY
$Revision: 5045 $
$Date: 2016-05-04 17:56:28 -0700 (Wed, 04 May 2016) $

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
    @ingroup LT3965
    Header for LT3965: Octal Matrix LED Bypass Switch
*/

//! @name LT3965 I2C 7-BIT DEVICE ADDRESS
//! @{
#define address1       0b00000000 // I2C Device address 1
#define address2       0b00001111   // I2C Device address 2

#define AC_ADDR_0      0x20
#define SC_ADDR_0      0x50
#define BC_ADDR        0x0C
#define TM_ADDR        0x73
//! @}

//! @name 8 CHANNELS
//! @{
#define CHANNEL0       0X00
#define CHANNEL1       0X01
#define CHANNEL2       0X02
#define CHANNEL3       0X03
#define CHANNEL4       0X04
#define CHANNEL5       0X05
#define CHANNEL6       0X06
#define CHANNEL7       0X07
//! @}

#define UP           1
#define DOWN       0

#define TRUE         1
#define FALSE        0

#define ALERTPIN     2
#define PWMPIN       7
#define LOADPIN      9
#define BUTTONPIN    12
#define REDLED       11

//! ACMODE Write Command to write a "value" byte to device at "address"
int8_t i2c_acwrite(uint8_t address, uint8_t value);

//! ACMODE Read Command to read 3 data bytes from device at "address"
int8_t i2c_acread(uint8_t address, uint8_t *values);

//! SCMODE Write Command Short Format to write a "value" byte to device at "address"
int8_t i2c_scwriteshort(uint8_t address, uint8_t channel, uint8_t data_SCMREG);

//! SCMODE Write Command Long Format to write 2 "value" bytes to device at "address"
int8_t i2c_scwritelong(uint8_t address, uint8_t channel, uint8_t data_SCMREG, uint8_t dimming_value);

//! SCMODE Write Short + SCMODE Read Command
int8_t i2c_scwriteshort_scread(uint8_t address, uint8_t channel, uint8_t data_SCMREG, uint8_t *values);

//! BCMODE Read Command
int8_t i2c_bcread(uint8_t *value);