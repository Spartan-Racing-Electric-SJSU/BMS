#include <Arduino.h>
#include <SoftwareSerial.h>
#define LOAD_RESISTOR 10000

int currentSensorValue = 0; 
int voltageReferenceValue = 0;
float voltageA0 = 0;				
float voltageA1 = 0;	
float currentA0 = 0;   

void setup()
{
  	Serial.begin(9600);              //  setup serial
}

void loop()
{  
    currentSensorValue = analogRead(A0);        // variable to store the value read	
  	Serial.print("Raw value output: ");
	Serial.println(currentSensorValue);
	delay(50);
	voltageA0 = currentSensorValue * (5.0 / 1023.0);
	Serial.print("Voltage A0 OUT: ");
	Serial.println(voltageA0, 10);             				// debug value
	delay(50);
	currentA0 = voltageA0 / LOAD_RESISTOR;
	Serial.print("Current: ");
	Serial.println(currentA0, 10);	

	voltageReferenceValue = analogRead(A1);
	Serial.print("Raw value VREF: ");
	Serial.println(voltageReferenceValue);
	voltageA1 = voltageReferenceValue * (5.0 / 1023.0);
	Serial.print("Voltage A1 VREF: ");
	Serial.println(voltageA1, 10);             				// debug value
	delay(50);
}