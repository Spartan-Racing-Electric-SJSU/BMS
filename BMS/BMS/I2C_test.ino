#include <Arduino.h>
#include <stdint.h> 
#include <SoftwareSerial.h>
#include <Linduino.h> 
#include "LT_Wire.h"

// Global object declaration
LT_TwoWire i2c;

void setup()
{    
    i2c.begin(10000);
}

void loop()
{
    // // Master controls I2C bus
    // // Master and Slave know the struct to transfer
    // uint8_t buffer[10];
    // buffer[0] = 0x00;
    // buffer[1] = 0x14;
    // i2c.write( buffer, 10);

    // Request data from Slave, wait, place data into buffer, return number of bytes 
    uint8_t buf[10];
    int n = i2c.requestFrom(0x00, buf, 10);
    Wire.readBytes( buf, n);
    if ( n != 10)
    Serial.println("Error, didn't receive 10 bytes");
}
