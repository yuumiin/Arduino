//Pins
int outputPin = 9;
int analogPin = A0; //Analog pin for temp read


//Variables
float temperature_read = 0.0;
float set_temperature = 100;
float PID_error = 0;
float previous_error = 0;
float elapsedTime, Time, timePrev;
int PID_value = 0;

float Temp;               //Temperature
float Vref;               //Referent voltage
float Vout;               //Voltage after adc
float value;              //Sensor value

//PID constants
int kp = 3;   int ki = 0;   int kd = 0;
int PID_p = 0;    int PID_i = 0;    int PID_d = 0;

int run;
float data[50];

void setup() {
  Serial.begin(115200);
  pinMode(outputPin, OUTPUT);
//  TCCR2B = TCCR2B & B11111000 | 0x03;    // pin 3 and 11 PWM frequency of 980.39 Hz
  Time = millis();
  delay(1000);
}


void loop() {
  Serial.print("setTemp:");  Serial.print(set_temperature);  Serial.print(",");
  Serial.print("Temp:");     Serial.print(temperature_read);     Serial.print(",");
  Serial.print("Output:");    Serial.print(255-PID_value);    Serial.print(","); // P.S에 뜨는값
  Serial.println(",");

  temperature_read = readThermocouple();
  PID_error = set_temperature - temperature_read;
  PID_p = kp * PID_error;
  
  if (-3 < PID_error < 3)
  {
    PID_i = PID_i + (ki * PID_error);
  }

  timePrev = Time;                            
  Time = millis();                            
  elapsedTime = (Time - timePrev) / 1000;
  PID_d = kd * ((PID_error - previous_error) / elapsedTime);
  PID_value = PID_p + PID_i + PID_d;

  //We define PWM range between 0 and 255
  PID_value = constrain(PID_value, 0, 255);
  analogWrite(outputPin, 255-PID_value); //PULLUP이라 0을 출력하면 powerSupply에 전류흐름 오실로스코프에 뜨는 값은 0 서플라이에 뜨는건 5 이렇게
  previous_error = PID_error;     
  delay(300);
}

double readThermocouple() {
    value = analogRead(analogPin); //Analog value from temperature

    Vref = (value * 5.0) / 1024.0; //Conversion analog to digital for referent value
    Vout = (value  * 5.0) / 1024.0; //Conversion analog to digital for the temperature read voltage
//    Temp = (Vout - Vref) / 0.005; //Temperature calculation
    Temp = (Vout - 1.23) / 0.005;
    delay(300);
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
  kp = data[1];
  ki = data[2];
  kd = data[3];
  set_temperature = data[4];
}