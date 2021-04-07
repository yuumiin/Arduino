#include <Wire.h>
#include <Adafruit_ADS1X15.h>

#define DO 3
#define CS 4
#define CLK 5

Adafruit_ADS1115 ads;

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

float Temp;               //Temperature
float Vref;               //Referent voltage
float Vout;               //Voltage after adc
float value;              //Sensor value

int run; //DC supply on/off
float data[5];
int16_t adc0;


void setup() {
  Serial.begin(115200);
  ads.begin();
  pinMode(outputPin, OUTPUT);
  delay(100);
}

void loop()
{
  input_prameter();
  temperature_read = readThermocouple();
  currentTime = millis(); // 현재 시간
  elapsedTime = currentTime - previousTime; // 수행시간

// 제어하고자 하는 값(input)과 설정값을 비교하여 오차계산
  error = setPoint - input; // 오차 (설정값 - 입력값)

  cumError += error * elapsedTime; // I, 시간에 대한 누적오차
  rateError = (error - lastError) / elapsedTime; // D, 에러의 변화율

  double result = Kp * error + Ki * cumError + Kd * rateError; // PID
  
  result = constrain(result, 0, 255);

  if (run == 1) {
    analogWrite(9, 255 - result);
  }
  else if (run == 0) {
    analogWrite(9, 255);
  }

  lastError = error;
  previousTime = currentTime;
  delay(100);
}

double readThermocouple() {
  value = ads.readADC_SingleEnded(0);
  Vout = value * 0.1875 / 1000;
  Temp = (Vout - 1.23) / 0.005;
  delay(100);
  return Temp;
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