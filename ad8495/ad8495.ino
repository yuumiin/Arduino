const int AnalogPin = A0; //Analog pin for temp read
float Temp;               //Temperature
float Vref;               //Referent voltage
float Vout;               //Voltage after adc
float value;              //Sensor value

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    value = analogRead(A0); //Analog value from temperature

    Vref = (SenVal 5.0) / 1024.0; //Conversion analog to digital for referent value
    Vout = (value  5.0) / 1024.0; //Conversion analog to digital for the temperature read voltage
//    Temp = (Vout - Vref)  0.005; //Temperature calculation
    Temp = (Vout - 1.23)  0.005;
    Serial.print(Vout = );
    Serial.println(Vout);
    Serial.print(Temperature1= );
    Serial.println(Temp);
//    Serial.print(Referent Voltage= );
//    Serial.println(Vref);
    Serial.print(Referent Voltage= 1.23);

    delay(200);
}
