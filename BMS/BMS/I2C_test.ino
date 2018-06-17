#include <Arduino.h>
#include <stdint.h> 
#include <SoftwareSerial.h>
#include <Linduino.h> 
#include "LT_Wire.h"

// Global object declaration
LT_TwoWire i2c;
const int LT4316_READ_ADDRESS = 0b00000000; // Slave Address of LT4316 Resistive Dividers

#define POWER_REGISTER 0x00 // Power configuration  
#define LTC2451_TEMP_READ_ADDRESS 0x14 // Temperature Read address 

int D0, D_OUT; 

void setup()
{    
    i2c.begin(10000); // initiate LT_TwoWire library 
    Serial.begin(9600);
    delay(100);
    // Enable measure
    i2c.beginTransmission(LT4316_READ_ADDRESS);
    i2c.write(POWER_REGISTER);
    // Bit HIGH for enable measure
    i2c.write(8);
    i2c.endTransmission();
}

void loop()
{
    // Start Bit 
    // Slave Address 8-bit 
    // Ack 
    // Internal Register Address 8-bit 
    // Ack 
    // Data to send 8-bits 
    // Ack 
    // Stop bit 

    /**
     * LTC4316 I2C address              000_0000
     * LTC4316 Output Register Address 
     * ADM3260 SDA address 
     * ADM3260 SCL Address 
     * ADM3260 Output Register Address 
     * LTC2451 SCL address 
     * LTC2451 SDA address 
     * LTC2451 Temp + Address
     * LTC2451 Temp - Address 
     * LTC2451 Start Bits               0010100
     * LTC2451 Command bit              1 for Read, 0 for Write 
     */ 

    // Start transmitting
    i2c.beginTransmission(LT4316_READ_ADDRESS);
    // Specify Registers for data
    i2c.write(LTC2451_TEMP_READ_ADDRESS); 
    // End I2C bus transmitting and transmit from registers 
    i2c.endTransmission();
    // Request data from Slave, wait, place data into buffer, return number of bytes 
    int buffer_size = 2;
    uint8_t buf[buffer_size];
    int n = i2c.requestFrom(LT4316_READ_ADDRESS, buf, buffer_size);
    if(i2c.available() <= buffer_size)
    {
        D0 = i2c.readBytes(buf, n);
    }
    if ( n != buffer_size) Serial.println("Error, didn't receive 2 bytes");
    Serial.print("LTC2451 Read Data = ");
    Serial.print(D0);
    Serial.print("  LT4316 Data Out = ");
    Serial.println(D_OUT);
}
