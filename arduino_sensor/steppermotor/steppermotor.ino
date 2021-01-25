
int  step_revolution = 1600;

void setup() {
  // put your setup code here, to run once:
  pinMode(6, OUTPUT); //pul
  pinMode(5,  OUTPUT); //dir
  digitalWrite(5, LOW);
//  tone(6,100);
  Serial.begin(9600);
}

void loop() {
//  if (Serial.available()) {
//    char data = Serial.read();
//    if (data == '1') {
//      Serial.println('a');
//      noTone(6);
//      delay(1000);
//    }
//  }
//
//  //모터 시계방향으로 1회전
  digitalWrite(5, HIGH);

  for (int i = 0; i < step_revolution; i++) {
    digitalWrite(6, HIGH);
    delayMicroseconds(200);
    digitalWrite(6, LOW);
    delayMicroseconds(200);
  }

  delay(1000);

//  시계 반대 방향으로 5회전하려면 *5
  digitalWrite(5, LOW); //모터 방향 시계반대

  for (int i = 0; i < step_revolution; i++) {
    digitalWrite(6, HIGH);
    delayMicroseconds(100);
    digitalWrite(6, LOW);
    delayMicroseconds(100);
  }

  delay(1000);

}
