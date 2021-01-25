#include <SoftwareSerial.h>

SoftwareSerial rx(9, 8); //(rx, tx)
float c;

void setup() {
  Serial.begin(9600);
  rx.begin(9600);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  digitalWrite(5, HIGH);
}

void loop() {
  if (rx.available()) {
    c = rx.parseFloat();
    Serial.println(c);

    tone(6, 1000);

    if (c <= 0.03) {
      digitalWrite(5, LOW);
      tone(6, 1000);
      Serial.println("a");
    }

    if (c >= 1.5) {
      digitalWrite(5, HIGH);
      tone(6, 1000);
      Serial.println("b");
    }
  }
}
