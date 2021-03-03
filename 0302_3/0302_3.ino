int read_pin1 = 13; // 다른스위치핀은 gnd, 뒤
int read_pin2 = 12; // 앞
int state1;
int state2;
int pulse_pin = 5;
int dir_pin = 4;
char input;

#include <AccelStepper.h>
AccelStepper stepper(1, pulse_pin, dir_pin);

void setup() {
  Serial.begin(115200);
  pinMode(read_pin1, INPUT_PULLUP);
  pinMode(read_pin2, INPUT_PULLUP);
  pinMode(pulse_pin, OUTPUT);
  pinMode(dir_pin, OUTPUT);
  pinMode(7, OUTPUT);
  stepper.setMaxSpeed(14000);
  stepper.setAcceleration(10000);
}

void loop() {
  state1 = digitalRead(read_pin1);
  state2 = digitalRead(read_pin2);

  input = Serial.read();

  if (state1 == LOW || state2 == LOW) {
    if (state1 == LOW) {
      Serial.print("state1: ");
      Serial.println("close");
    }
    if (state2 == LOW) {
      Serial.print("state2: ");
      Serial.println("close");
    }
    stepper.stop();
    noTone(pulse_pin);
  }
  if (input == '1') {
    Serial.println(input);
    stepper.setMaxSpeed(20000);
    stepper.setAcceleration(15000);
    stepper.moveTo(20000); // 목표 위치
    int a = stepper.currentPosition();
    Serial.println(a);
  }
  if (input == '2') {
    stepper.setMaxSpeed(20000);
    stepper.setAcceleration(15000);
    stepper.moveTo(0);
    int b = stepper.currentPosition(); // 현재 위치값
    Serial.println(b);

    if (state1 == LOW) {
      Serial.println("close");
      stepper.stop();
      stepper.setCurrentPosition(0); // 현재 위치값 지정
    }
  }
  if (input == '3') { //뒤
    digitalWrite(dir_pin, LOW);
    tone(pulse_pin, 1000);
  }
  if (input == '4') {
    digitalWrite(dir_pin, HIGH);
    tone(pulse_pin, 1000);
  }
  
  stepper.run();

  //  // 왔다갔다
  //  if (state1 == LOW && state2 == HIGH) {
  //    Serial.println("앞");
  //    digitalWrite(dir_pin, HIGH);
  //    stepper.setMaxSpeed(14000);
  //    stepper.setAcceleration(10000);
  //    stepper.moveTo(8000);
  //    int a = stepper.currentPosition();
  //    stepper.run();
  //    Serial.println(a);
  //  }
  //
  //  else if (state2 == LOW && state1 == HIGH) {
  //    Serial.println("뒤");
  //
  //    digitalWrite(dir_pin, LOW);
  //    stepper.setMaxSpeed(14000);
  //    stepper.setAcceleration(10000);
  //    stepper.moveTo(-7000);
  //    stepper.run();
  //    //    noTone(pulse_pin);
  //
  //  }
}
