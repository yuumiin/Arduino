////////////////////////////////////////////값 전송///////////////////////////////////
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <SoftwareSerial.h>

SoftwareSerial tx(9, 8); //(rx(신호가 들어오는 곳, tx(신호가 나가는 곳))

Adafruit_ADS1115 ads(0x48);

void setup() {
  Serial.begin(9600);
  tx.begin(9600);
  ads.begin();
}

void loop() {
  int16_t adc1;
  float location;
  adc1 = ads.readADC_SingleEnded(1);
  location = adc1 * 0.1875 / 1000;
  tx.println(location);
  Serial.println(location);
  delay(100); //0.1초에 한번 값 전송

  
  
}
