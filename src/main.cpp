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

double reading;

long zero_factor_ch_a = -96400;

float calibration_ch_a = -20.0 * 0.35274 / 16.0f;
float calibration_ch_b = -40.0f * 0.35274 / 16.0f; // Guess

void get_readings() {
  
}

float calibration_factor = -20; //-7050 worked for my 440lb max scale setup
float units;
float ounces;
float lbs;

void setup() {
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");

  scale.begin(DOUT, CLK);

  scale.set_scale();
  scale.tare();	//Reset the scale to 0
  scale.set_offset(zero_factor);
}

void loop() {
  delay(100);
  scale.set_scale(calibration_factor); //Adjust to this calibration factor

  Serial.print("Reading: ");
  units = scale.get_units(4);
//  if (units < 0) {
//    units = 0.00;
//  }

  Serial.print(lbs, 3);
  Serial.print("lbs ");
  Serial.print(ounces, 3); 
  Serial.print("oz cf: ");
  Serial.print(calibration_factor);
  Serial.println();

  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 1;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 1;
  }
}