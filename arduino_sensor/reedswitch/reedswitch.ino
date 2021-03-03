int reed_pin = 13; // 다른스위치핀은 gnd
int state;
int pulse_pin = 5;
int dir_pin = 4;

void setup() {
  Serial.begin(115200);
  pinMode(read_pin, INPUT_PULLUP);
  pinMode(pulse_pin, OUTPUT);
  pinMode(dir_pin, OUTPUT);
  pinMode(7, OUTPUT);

}

void loop() {
  state = digitalRead(read_pin);
  
  if (state == HIGH) {
    Serial.println("open");

    tone(pulse_pin, 1000);
  }
  else {
    Serial.println("close");
    noTone(pulse_pin);
  }
}
