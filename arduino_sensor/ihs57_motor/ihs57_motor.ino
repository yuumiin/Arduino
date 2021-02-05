int count = 0;
void setup() {
  Serial.begin(115200);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);

  tone(5,500);
  count++;
  Serial.println(count);
  

}

void loop() {
  //  digitalWrite(5, HIGH);
  //  tone(6, 3200);
  //    for (int i = 0; i < 3200; i++) {
  //    digitalWrite(5, HIGH);
  //    //1ms(1000us) 대기
  //    delayMicroseconds(1000);
  //    //OFF 1.8의 1/4 움직임
  //    digitalWrite(5, LOW);
  //    //1ms(1000us) 대기
  //    delayMicroseconds(1000);
  //  }

  //if(Serial.available()) {
  //  char a = Serial.read();
  //  Serial.println(a);
  //  if(a == '1') {
  //    noTone(5);
  //  }
  //}
}
