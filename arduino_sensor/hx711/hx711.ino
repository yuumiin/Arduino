#include "HX711.h"
#define DOUT 3
#define SLK 2
HX711 scale;

void setup() {
  Serial.begin(9600);
  scale.begin(DOUT, SLK);
}

void loop() {
  int weight_value = scale.get_units(1);
  Serial.print(weight_value, 1);

  
  if (scale.is_ready()) {
    long reading = scale.read();
//    Serial.print("HX711 reading: ");
//    Serial.println(reading);
  }
  else {
    Serial.println("HX711 not found");
  }
  delay(1000);
}
