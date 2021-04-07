#include <SPI.h>
#include "Adafruit_MAX31855.h"

#define DO 3
#define CS 4
#define CLK 5

// gain 값
double Kp = 0; // 비례상수 (기준값과 현재값 오차에 kp 곱해 오차값 줄임)
double Ki = 0;
double Kd = 0;

unsigned long currentTime, previousTime;
double elapsedTime;
double error;
double lastError;
double output;
double setPoint = 0;
double cumError, rateError;
float temperature_read = 0.0;

int run; //DC supply on/off

float data[10]; // run, kp, ki, kd, setPoint

Adafruit_MAX31855 thermocouple(CLK, CS, DO);

void setup() {
  Serial.begin(115200);

  while (!Serial)
    delay(1);

  // Serial.println("MAX31855 test");
  // wait for MAX chip to stabilize
  delay(1000);
  // Serial.print("Initializing sensor...");
  if (!thermocouple.begin())
  {
    Serial.println("Thermocouple ERROR");
    while (1)
      delay(10);
  }
  // Serial.println("DONE.");
  pinMode(9, OUTPUT);
}

void loop() {
  input_prameter();
  temperature_read = ReadTemperature();
  output = PIDControl(temperature_read);
  delay(500);
}

double ReadTemperature()
{
  // Serial.print("Internal Temp = ");
  // Serial.println(thermocouple.readInternal());

  double celsius = thermocouple.readCelsius();
  if (isnan(celsius))
  {
    Serial.println("Something wrong with thermocouple!");
  }
  else
  {
    // Serial.print("C = ");
    Serial.print(celsius);
    Serial.print(",");
  }
  return celsius;
}

double PIDControl(double input) {
  // Serial.print("Kp : "); Serial.print(Kp); Serial.print(" , Ki : "); Serial.print(Ki); Serial.print(" , Kd : "); Serial.print(Kd);
  currentTime = millis(); // 현재 시간
  elapsedTime = currentTime - previousTime; // 수행시간
  // Serial.print("current Time = "); Serial.print(currentTime); Serial.print(" , 경과시간 = "); Serial.println(elapsedTime);

  // 제어하고자 하는 값(input)과 설정값을 비교하여 오차계산
  error = setPoint - input; // 오차 (설정값 - 입력값)

  cumError += error * elapsedTime; // I, 시간에 대한 누적오차
  rateError = (error - lastError) / elapsedTime; // D, 에러의 변화율

  double result = Kp * error + Ki * cumError + Kd * rateError; // PID
  
  result = constrain(result, 0, 255);

  if (run == 1) {
    analogWrite(9, result);
  }

  lastError = error;
  previousTime = currentTime;

//   Serial.print(" // error = "); Serial.print(error);
//   Serial.print(" P : "); Serial.print(Kp * error);
//   Serial.print(" I : "); Serial.print(Ki * cumError);
//   Serial.print(" D : "); Serial.print(Kd * rateError);
//   Serial.print(" pid = "); Serial.print(Kp * error + Ki * cumError + Kd * rateError);
//   Serial.print(" analogWrite input = "); Serial.println(result);

  return result;
}

void input_prameter() {
  if (Serial.available()) {
    String content = Serial.readStringUntil('\n');
    int content_length = content.length();
    for (int i = 0; i < content_length; i++)
    {
      int index = content.indexOf(",");
      data[i] = atof(content.substring(0, index).c_str());
      content = content.substring(index + 1);
    }
  }

  run = data[0];
  Kp = data[1];
  Ki = data[2];
  Kd = data[3];
  setPoint = data[4];
}
