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

}
