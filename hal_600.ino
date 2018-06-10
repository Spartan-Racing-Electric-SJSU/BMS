#include <Arduino.h>
#include <SoftwareSerial.h>
#define LOAD_RESISTOR 1000
const float currentConsumption = 0.025; // 25mA

int currentSensorOutput0VAnalogRead = 0; // Pin Read for 0V
int currentSensorOutput4VAnalogRead = 0; // Pin Read for 4V
float currentSensorOutput0VRawValue = 0; // 0 Voltage converted through math after analog read				
float currentSensorOutput4VRawValue = 0; // 4 Voltage converted through math after analog read				
float currentCalculation = 0; // Use Ohm's law to find current

void setup()
{
  	Serial.begin(9600);              //  setup serial
}

void loop()
{  
	// // Charging Cart HASS 300-S
    // currentSensorOutput0VAnalogRead = analogRead(A0);        // variable to store the value read	
  	// Serial.print("Raw value output: ");
	// Serial.println(currentSensorOutput0VAnalogRead);
	// delay(50);
	// currentSensorOutput0VRawValue = currentSensorOutput0VAnalogRead * (5.0 / 1023.0);
	// Serial.print("Voltage A0 OUT: ");
	// Serial.println(currentSensorOutput0VRawValue, 10);             				// debug value
	// delay(50);
	// currentCalculation = currentSensorOutput0VRawValue / LOAD_RESISTOR;
	// Serial.print("Current: ");
	// Serial.println(currentCalculation, 10);	

	// currentSensorOutput5VAnalogRead = analogRead(A1);
	// Serial.print("Raw value VREF: ");
	// Serial.println(currentSensorOutput5VAnalogRead);
	// currentSensorOutput4VRawValue = currentSensorOutput5VAnalogRead * (5.0 / 1023.0);
	// Serial.print("Voltage A1 VREF: ");
	// Serial.println(currentSensorOutput4VRawValue, 10);             				// debug value
	// delay(50);

	// Master Penthouse HAL 200-S 
	// analogReadResolution(10); // 10 bit default 
	currentSensorOutput4VAnalogRead = analogRead(A0);        // variable to store the value read	
  	Serial.print("Raw value output: ");
	Serial.println(currentSensorOutput4VAnalogRead);
	delay(50);
	currentSensorOutput4VRawValue = currentSensorOutput4VAnalogRead * (5.0 / 1023.0);
	Serial.print("Voltage output: ");
	Serial.println(currentSensorOutput4VRawValue, 10);             				// debug value
	delay(50);
	currentCalculation = currentSensorOutput4VRawValue / LOAD_RESISTOR - currentConsumption;
	Serial.print("Current output: ");
	Serial.println(currentCalculation, 10);	

	currentSensorOutput0VAnalogRead = analogRead(A1);
	Serial.print("Raw value 0V: ");
	Serial.println(currentSensorOutput0VAnalogRead);
	currentSensorOutput0VRawValue = currentSensorOutput0VAnalogRead * (5.0 / 1023.0);
	Serial.print("Voltage 0V: ");
	Serial.println(currentSensorOutput0VRawValue, 10);             				// debug value
	delay(50);
}