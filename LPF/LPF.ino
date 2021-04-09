#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

int outputPin = 9;

double pre_value = 0;
double pre_time = 0;
double temperature;
int alpha = 170;
int PWM_value;

void setup() {
  Serial.begin(115200);
  ads.begin();
  temperature = readTemp();
}

void loop() {
  double st_time = millis();
  double dt = (st_time - pre_time);
  //temperature: measured value from a sensor (센서에서 측정된 값)
  temperature = readTemp();
  double lpf_value;

  lpf_value = alpha / (alpha + dt) * pre_value + dt / (alpha + dt) * temperature;

  Serial.print("temp:");Serial.print(temperature); Serial.print("\t");
  Serial.print("alpha_170:");Serial.print(lpf_value); Serial.print("\t");
  Serial.println(",");

  pre_value = lpf_value;
  pre_time = st_time;
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
