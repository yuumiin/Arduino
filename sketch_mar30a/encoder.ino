#define encoder0PinA 2
#define encoder0PinB 3

int encoder0Pos = 0;

void setup() {
  pinMode(encoder0PinA, INPUT); 
  pinMode(encoder0PinB, INPUT); 
  attachInterrupt(0, doEncoderA, CHANGE); // encoder pin on interrupt 0 (pin 2)
  attachInterrupt(1, doEncoderB, CHANGE); // encoder pin on interrupt 1 (pin 3)
  Serial.begin (115200);
}

void loop(){}

void doEncoderA(){
  // look for a low-to-high on channel A
  if (digitalRead(encoder0PinA) == HIGH) { 
    // check channel B to see which way encoder is turning
    if (digitalRead(encoder0PinB) == LOW) {  
      encoder0Pos = encoder0Pos + 1;
    } 
    else {
      encoder0Pos = encoder0Pos - 1;
    }
  }
  else   // must be a high-to-low edge on channel A                                       
  { 
    // check channel B to see which way encoder is turning  
    if (digitalRead(encoder0PinB) == HIGH) {   
      encoder0Pos = encoder0Pos + 1;
    } 
    else {
      encoder0Pos = encoder0Pos - 1;
    }
  }
  Serial.println (encoder0Pos);
}

void doEncoderB(){
  // look for a low-to-high on channel B
  if (digitalRead(encoder0PinB) == HIGH) {   
   // check channel A to see which way encoder is turning
    if (digitalRead(encoder0PinA) == HIGH) {  
      encoder0Pos = encoder0Pos + 1;
    } 
    else {
      encoder0Pos = encoder0Pos - 1;
    }
  }
  // Look for a high-to-low on channel B
  else { 
    // check channel B to see which way encoder is turning  
    if (digitalRead(encoder0PinA) == LOW) {   
      encoder0Pos = encoder0Pos + 1;
    } 
    else {
      encoder0Pos = encoder0Pos - 1;
    }
  }
  Serial.println (encoder0Pos);
}

/* #define outputA 2
#define outputB 3

int pulPin = 5;
int dirPin = 6;
int counter = 0;
int aState;
int aLastState;

void setup()
{
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);
  pinMode(pulPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  Serial.begin(115200);
  // Reads the initial state of the outputA
  aLastState = digitalRead(outputA);
}

void loop()
{
  aState = digitalRead(outputA); // Reads the "current" state of the outputA
  // If the previous and the current state of the outputA are different, that means a Pulse has occured
  if (aState != aLastState)
  {
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(outputB) != aState)
    {
      counter++;
    }
    else
    {
      counter--;
    }
    Serial.print("Position: ");
    Serial.println(counter);
  }
  aLastState = aState; // Updates the previous state of the outputA with the current state
}


 */