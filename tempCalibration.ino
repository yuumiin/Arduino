#include <SPI.h>
#include "Adafruit_MAX31855.h"

#define DO 3
#define CS 4
#define CLK 5
#define relayPin 9

Adafruit_MAX31855 thermocouple(CLK, CS, DO);

void setup() {
  Serial.begin(115200);
  pinMode(9, OUTPUT);
}

void loop() {
  // Serial.print("Internal Temp = ");
  // Serial.println(thermocouple.readInternal());
  for(int i = 0; i < 256; i++) {
      digitalWrite(9, i);
      float Vout = (value  * 5.0) / 1024.0;
      double celsius = thermocouple.readCelsius();
      Serial.println(digitalRead(9, HIGH));
      Serial.print(" , ");
      Serial.println(celsius);
      Serial.print(" , ");
      Serial.println(Vout);
  }
}