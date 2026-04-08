#include <Arduino.h>
#include <ColorSensors/spectrometer.h>
#include <debug.h>
#include <PosSensors/position.h>
#include <i2cScanner.h>
#include <Adafruit_NeoPixel.h>
#include <pins.h>

#include <config.h>

#define USE_scanner true
#if CAT(USE_, CURR_MAIN)
#undef USE_scanner

void rgbcSensorOnEnter() {
}

void rgbcSensorOnExit() {
}

I2CScanner scanner;

void setup() 
{	
	//uncomment the next line if you use custom sda and scl pins for example with ESP8266-01 (sda = 4, scl = 5)
	//Wire.begin(SDA_PIN, SCL_PIN);
	
	Serial.begin(BAUDE_RATE);

	while (!Serial) {};

	scanner.Init();
}

void loop() 
{
	scanner.Scan();
	delay(5000);
}

#endif