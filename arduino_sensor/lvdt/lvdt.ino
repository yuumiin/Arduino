#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads(0x48);

void setup() {
  Serial.begin(9600);
  ads.begin();
}

void loop() {
  int16_t adc1;
  adc1 = ads.readADC_SingleEnded(1);
//  adc1 = adc1*0.1875/1000;
  Serial.print("AINO: ");
  Serial.println(adc1*0.1875/1000);
  
//  delay(2000);
}
