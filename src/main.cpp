#include <Arduino.h>
#include <stdint.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "UserInterface.h"
#include "LTC68042.h"
#include "LTC6804_COMM.h"
#include <SPI.h>

#include "test_mode.h"
#include "gui_mode.h"


void setup(){
  Serial.begin(115200);
  LTC6804_initialize();  //Initialize LTC6804 hardware
  init_cfg();        //initialize the 6804 configuration array to be written

}

void loop(){
  if (Serial.available())           // Check for user input
  {
    Serial.println("Serial.available() == true");
    uint32_t user_command;
    user_command = read_int();      // Read the user command
    Serial.println(user_command);
    switch(user_command){
      case 8:
        COMM_REG reg;
        reg.fields.ICOM0 = 0x6;
        reg.fields.D0 = 0xA0;
        reg.fields.FCOM0 = 0x8;
        reg.fields.ICOM1 = 0x0;
        reg.fields.D1 = 0x01;
        reg.fields.FCOM1 = 0x8;
        reg.fields.ICOM2 = 0x0;
        reg.fields.D2 = 0xAA;
        reg.fields.FCOM2 = 0x9;

        LTC6804_wrcomm(1, reg.bytes)

        reg.fields.ICOM0 = 0x6;
        reg.fields.D0 = 0xA0;
        reg.fields.FCOM0 = 0x8;
        reg.fields.ICOM1 = 0x0;
        reg.fields.D1 = 0x01;
        reg.fields.FCOM1 = 0x8;
        reg.fields.ICOM2 = 0x0;
        reg.fields.D2 = 0xAA;
        reg.fields.FCOM2 = 0x9;
                
        LTC6804_stcomm(1, reg.bytes);

        reg.fields.ICOM0 = 0x6;
        reg.fields.D0 = 0xA0;
        reg.fields.FCOM0 = 0x8;
        reg.fields.ICOM1 = 0x0;
        reg.fields.D1 = 0x01;
        reg.fields.FCOM1 = 0x8;
        reg.fields.ICOM2 = 0x0;
        reg.fields.D2 = 0xAA;
        reg.fields.FCOM2 = 0x9;

        LTC6804_rdcomm(1, reg.bytes);


        break;

      default:
        run_command(user_command);
        break;
    }
  }
}
