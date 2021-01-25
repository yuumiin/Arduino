#include <HX711.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <SoftwareSerial.h>
#include "HX711.h"
HX711 scale;
Adafruit_ADS1115 ads(0x48);

//connected pin
int pulse_pin = 5;
int dir_pin = 4;
int enable_pin = 6;

//hx711 variable
int dt_pin = 2;
int sck_pin = 3;

//position variable
int16_t adc1;
float location;

//loadcell variable
float weight; //measure loadcell value
float kgf_weight; //change weight unit
float base_value; //set value
float base_weight; //change input_weight unit

//calibration variable
float calibration_factor = 2063000; //9452528 2063000
float current_calibration;
//motor control
bool enable = false;
int motor_frequency = 1000;

//read labview input value
String input;
String button;
String input_weight; //labview input weight
char mode; //change motor mode

void setup() {
  Serial.begin(115200);
  ads.begin();

  pinMode(pulse_pin, OUTPUT);
  pinMode(dir_pin, OUTPUT);
  pinMode(enable_pin, OUTPUT);
  digitalWrite(enable_pin, LOW);

  scale.begin(dt_pin, sck_pin);
  scale.set_scale(calibration_factor); //calibration
}

void loop() {
  if (Serial.available()) {
    mode = Serial.read();
  }
  
  switch (mode) {
    case 'c':
      calibration();
      break;
    case 'm':
      read_value();
      motor_control();
      break;
  }
}

void read_value() {
  adc1 = ads.readADC_SingleEnded(1);
  location = adc1 * 0.1875 / 1000;

  weight = scale.get_units();
  kgf_weight = weight * 9.8;

  Serial.print(location);
  Serial.print(',');
  Serial.println(kgf_weight, 3);
}

void motor_control() {
  input = Serial.readStringUntil('\n');
  int first = input.indexOf(",");
  int second = input.indexOf(",", first + 1);
  button = input.substring(0, first); // 1
  input_weight = input.substring(first + 1, second); // 0.2
  base_weight = input_weight.toFloat() * 9.8;

  if (button == "1") {
    Serial.println("on");
    base_value = base_weight;
    enable = true;
  }
  if (button == "2") {
    Serial.println("off");
    noTone(pulse_pin);
    enable = false;
  }
  if (button == "3") {
    Serial.println("Loadcell Set to Zero");
    scale.tare();
  }
  if (button == "c") {
    mode = 'c';
  }

  if (enable) {
    Serial.println("enable");
    Serial.print("base_value: ");
    Serial.println(base_value);
    if (kgf_weight < base_value * 1 / 10) 
    {digitalWrite(dir_pin, HIGH);tone(pulse_pin, 800);}
    if (kgf_weight >= base_value * 1 / 10 && kgf_weight < base_value * 2 / 10) 
    {digitalWrite(dir_pin, HIGH);tone(pulse_pin, 700);}
    if (kgf_weight >= base_value * 2 / 10 && kgf_weight < base_value * 3 / 10) 
    {digitalWrite(dir_pin, HIGH);tone(pulse_pin, 600);}
    if (kgf_weight >= base_value * 3 / 10 && kgf_weight < base_value * 4 / 10) 
    {digitalWrite(dir_pin, HIGH);tone(pulse_pin, 500);}
    if (kgf_weight >= base_value * 5 / 10 && kgf_weight < base_value * 6 / 10) 
    {digitalWrite(dir_pin, HIGH);tone(pulse_pin, 400);}
    if (kgf_weight >= base_value * 6 / 10 && kgf_weight < base_value * 7 / 10) 
    {digitalWrite(dir_pin, HIGH);tone(pulse_pin, 300);}
    if (kgf_weight >= base_value * 7 / 10 && kgf_weight < base_value * 8 / 10) 
    {digitalWrite(dir_pin, HIGH);tone(pulse_pin, 200);}
    if (kgf_weight >= base_value * 8 / 10 && kgf_weight < base_value * 9 / 10) 
    {digitalWrite(dir_pin, HIGH);tone(pulse_pin, 100);}
    if (kgf_weight >= base_value * 9 / 10 && kgf_weight < base_value * 10 / 10) 
    {digitalWrite(dir_pin, HIGH);tone(pulse_pin, 80);}
    if (kgf_weight >= base_value && kgf_weight < base_value + 0.001) 
    {Serial.println("Set Weight!");noTone(pulse_pin);enable = false;}
    if (kgf_weight >= base_value + 0.001) 
    {digitalWrite(dir_pin, LOW); tone(pulse_pin, 80);}
  }
  else {
    if(location > 2.00) {
      noTone(pulse_pin);
    }
    if (button == "4") {
        digitalWrite(dir_pin, LOW);
        tone(pulse_pin, 800);
    }
    if (button == "5") {
      Serial.println("start");
      digitalWrite(dir_pin, HIGH);
      tone(pulse_pin, motor_frequency);
    }
    if (button == "6") {
      Serial.println("down");
      digitalWrite(dir_pin, HIGH);
      tone(pulse_pin, motor_frequency, 1000);
    }
    if (button == "7") {
      Serial.println("up");
      digitalWrite(dir_pin, LOW);
      tone(pulse_pin, motor_frequency, 1000);
    }
    if (button == "8") {
      Serial.println("down");
      digitalWrite(dir_pin, HIGH);
      tone(pulse_pin, motor_frequency, 500);
    }
    if (button == "9") {
      Serial.println("up");
      digitalWrite(dir_pin, LOW);
      tone(pulse_pin, motor_frequency, 500);
    }
  }
}

float calibration() {
  Serial.print(scale.get_units() * 9.8, 3);
  Serial.print(",");
  Serial.println(calibration_factor);

  input = Serial.readStringUntil('\n');
  if (input == "tare") {
    scale.tare();
  }
  if (input == "+10") {
    calibration_factor += 10;
  }
  if (input == "+100") {
    calibration_factor += 100;
  }
  if (input == "+1000") {
    calibration_factor += 1000;
  }
  if (input == "-10") {
    calibration_factor -= 10;
  }
  if (input == "-100") {
    calibration_factor -= 100;
  }
  if (input == "-1000") {
    calibration_factor -= 1000;
  }
  if (input == "end") {
    current_calibration = calibration_factor;
    Serial.print("Set calibration value: ");
    Serial.println(current_calibration);
    mode = 'm';
  }
}
