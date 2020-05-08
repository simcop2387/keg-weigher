#include <Arduino.h>
#include <HX711.h>



/*
 Started with example code written by Nathan Seidle from SparkFun Electronics and added
 LCD output with gram and ounce values.
 
 Setup your scale and start the sketch WITHOUT a weight on the scale
 Once readings are displayed place the weight on the scale
 Press +/- or a/z to adjust the calibration_factor until the output readings match the known weight

 Arduino pin 6 -> HX711 CLK
 Arduino pin 5 -> HX711 DOUT
 Arduino pin 5V -> HX711 VCC
 Arduino pin GND -> HX711 GND
 
 The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.
 
 The HX711 library can be downloaded from here: https://github.com/bogde/HX711
*/

#define DOUT D1
#define CLK  D2

HX711 scale;

const int averages = 4;

long zero_factor_ch_a = -70000;
long zero_factor_ch_b = -30000;

float calibration_ch_a = -171.75f / (0.35274f / 16.0f);
float calibration_ch_b = -400.0f / (0.35274f / 16.0f); // Guess

float keg_base_ch_a = 15.5f;
float keg_base_ch_b = 15.5f;

float reading_ch_a = 0.0;
float reading_ch_b = 0.0;
float water_ch_a = 0.0;
float water_ch_b = 0.0;
long  raw_ch_a = 0;
long  raw_ch_b = 0;

void get_readings() {
  // Get the readings
  scale.set_gain(128);
  raw_ch_a = scale.read_average(averages);
  scale.set_gain(32);
  raw_ch_b = scale.read_average(averages);

  // Scale appropriately
  reading_ch_b = ((float) raw_ch_b - zero_factor_ch_b) / calibration_ch_b;
  reading_ch_a = ((float) raw_ch_a - zero_factor_ch_a) / calibration_ch_a;
  
  // Calculate water weight
  water_ch_a = reading_ch_a - keg_base_ch_a;
  water_ch_a = water_ch_a > 0.0 ? water_ch_a : 0.0;

  water_ch_b = reading_ch_b - keg_base_ch_b;
  water_ch_b = water_ch_b > 0.0 ? water_ch_b : 0.0;

}

void setup() {
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");

  scale.begin(DOUT, CLK);
  scale.tare();
  scale.set_offset(zero_factor_ch_a);
}

void loop() {
  get_readings();

  Serial.print("Readings: ");
  Serial.print(reading_ch_a);
  Serial.print(" lbs, ");
  Serial.print(reading_ch_b);
  Serial.print(" lbs.  water: ");
  Serial.print(water_ch_a);
  Serial.print(" lbs, ");
  Serial.print(water_ch_b);
  Serial.print("lbs  Raw: ");
  Serial.print(raw_ch_a);
  Serial.print(", ");
  Serial.print(raw_ch_b);
  Serial.println();

}