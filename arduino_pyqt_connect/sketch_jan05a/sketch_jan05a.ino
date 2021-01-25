#include <Servo.h>
Servo servo;
int motor = 9;
char value;

void setup() {
  Serial.begin(9600);
  servo.attach(motor);
  servo.write(0);
}

void loop() {
  while(Serial.available()) {
    value = Serial.read();
    
    if(value == '1') {
      for(int i = 0; i < 180; i++) {
        servo.write(i);
        delay(10);
      }
    }
    else if (value == '0') {
      servo.write(0);
    }
  }
}
