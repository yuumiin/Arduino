#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads(0x48);

void setup() {
  Serial.begin(9600);
  ads.begin();
}

void loop() {
  int16_t adc0, adc1, adc2, adc3;
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);
  Serial.print("AINO: ");
  Serial.println(adc0*0.1875/1000);
  Serial.print("AIN1: ");
  Serial.println(adc1*0.1875/1000);
  Serial.print("AIN2: ");
  Serial.println(adc2*0.1875/1000);
  Serial.print("AIN3: ");
  Serial.println(adc3*0.1875/1000);

  delay(3000);
}
