#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include "HX711.h"

#define DOUT 3 // hx711 데이터 아웃 핀 넘버 3
#define SLK 2 // 클락 핀 넘버

Adafruit_ADS1115 ads(0x48);
HX711 scale;

int step_revolution = 1600;
bool stop = false; //작동중

void setup() {
  Serial.begin(9600);
  ads.begin();
  scale.begin(DOUT, SLK);
  pinMode(6, OUTPUT); //pul
  pinMode(5,  OUTPUT); //dir
}

void loop() {
  //위치
  int16_t adc1;
  adc1 = ads.readADC_SingleEnded(1);

  //로드셀
  if (scale.is_ready()) {
    long loadcell = scale.read();
    Serial.print(adc1 * 0.1875 / 1000);
    Serial.print(",");
    Serial.print(loadcell);
    Serial.println("");
  }
  else {
    Serial.println("HX711 not found");
  }
  delay(100);

//  motor();
  if (Serial.available()) {
    char start = Serial.read();
    if (start == '1') {
      Serial.println('a');
      motor();
    }
  }

}

void motor() {
  digitalWrite(5, HIGH);
  for (int i = 0; i < step_revolution; i++) {
    digitalWrite(6, HIGH);
    delayMicroseconds(200);
    digitalWrite(6, LOW);
    delayMicroseconds(200);
  }
  //시계 반대 방향
  digitalWrite(5, LOW); //모터 방향 시계반대
  for (int i = 0; i < step_revolution; i++) {
    digitalWrite(6, HIGH);
    delayMicroseconds(200);
    digitalWrite(6, LOW);
    delayMicroseconds(200);
  }

}
