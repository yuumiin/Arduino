#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

int outputPin = 9;

double pre_value = 0;
double pre_time = 0;
double temperature;
int alpha = 170;
int PWM_value;
double lpf_value;

void setup() {
  Serial.begin(115200);
  ads.begin();
  pre_value = readTemp();
}

void loop() {
  double current_time = millis();
  double dt = (current_time - pre_time);
  temperature = readTemp();
  lpf_value = alpha / (alpha + dt) * pre_value + dt / (alpha + dt) * temperature;

  Serial.print(lpf_value);Serial.print(",");

  pre_value = lpf_value;
  pre_time = current_time;
  delay(100);

  PWMControl();
}

double readTemp() {
  float value = ads.readADC_SingleEnded(0);
  float Vout = value * 0.1875 / 1000;
  float Temp = (Vout - 1.23) / 0.005;
  return Temp;
}
void PWMControl() {
  if (Serial.available()) {
    String value = Serial.readStringUntil('\n');
    PWM_value = value.toInt();
    analogWrite(outputPin, 255 - PWM_value);
  }
}

/*
참고
https://pinkwink.kr/978
https://pinkwink.kr/931
https://sw-ryu0405.blogspot.com/2019/12/low-pass-filter-lpf-arduino-code.html
*/
