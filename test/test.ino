/******************************************************************************
   AutoTune RC Filter Example
   Use Serial Monitor and Serial Plotter to view results.
   Reference: https://github.com/Dlloydev/QuickPID/wiki/AutoTune
   Circuit: https://github.com/Dlloydev/QuickPID/wiki/AutoTune_RC_Filter

   TUNING RULE             RECOMMENED FOR
   0 ZIEGLER_NICHOLS_PI    Good noise and disturbance rejection
   1 ZIEGLER_NICHOLS_PID   Good noise and disturbance rejection
   2 TYREUS_LUYBEN_PI      Time-constant (lag) dominant processes (conservative)
   3 TYREUS_LUYBEN_PID     Time-constant (lag) dominant processes (conservative)
   4 CIANCONE_MARLIN_PI    Delay (dead-time) dominant processes
   5 CIANCONE_MARLIN_PID   Delay (dead-time) dominant processes
   6 AMIGO_PID             More universal than ZN_PID (uses a dead time dependancy)
   7 PESSEN_INTEGRAL_PID   Similar to ZN_PID but with better dynamic response
   8 SOME_OVERSHOOT_PID    ZN_PID with lower proportional and integral gain
   9 NO_OVERSHOOT_PID      ZN_PID with much lower P,I,D gain settings
 ******************************************************************************/

#include "QuickPID.h"

const byte inputPin = A0;
const byte outputPin = 3;

int Print = 0;                // on(1) monitor, off(0) plotter
int tuningRule = 1;           // see above table
float POn = 1.0;              // Mix of PonE to PonM (0.0-1.0)
unsigned long timeout = 120;  // AutoTune timeout (sec)

int Input, Output, Setpoint;
float Kp = 0, Ki = 0, Kd = 0;

QuickPID myQuickPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, POn, DIRECT);

void setup()
{
  Serial.begin(115200);
  myQuickPID.AutoTune(inputPin, outputPin, tuningRule, Print, timeout);
  myQuickPID.SetTunings(myQuickPID.GetKp(), myQuickPID.GetKi(), myQuickPID.GetKd());
  myQuickPID.SetSampleTimeUs(5000); // recommend 5000µs (5ms) minimum
  myQuickPID.SetMode(AUTOMATIC);
  Setpoint = 700;

  if (Print == 1) {
    // Controllability https://blog.opticontrols.com/wp-content/uploads/2011/06/td-versus-tau.png
    if (float(myQuickPID.GetTu() / myQuickPID.GetTd() + 0.0001) > 0.75) Serial.println("This process is easy to control.");
    else if (float(myQuickPID.GetTu() / myQuickPID.GetTd() + 0.0001) > 0.25) Serial.println("This process has average controllability.");
    else Serial.println("This process is difficult to control.");
    Serial.print("Tu: "); Serial.print(myQuickPID.GetTu());    // Ultimate Period (sec)
    Serial.print("  td: "); Serial.print(myQuickPID.GetTd());  // Dead Time (sec)
    Serial.print("  Ku: "); Serial.print(myQuickPID.GetKu());  // Ultimate Gain
    Serial.print("  Kp: "); Serial.print(myQuickPID.GetKp());
    Serial.print("  Ki: "); Serial.print(myQuickPID.GetKi());
    Serial.print("  Kd: "); Serial.println(myQuickPID.GetKd());
    delay(6000);
  }
}

void loop()
{ // plotter
  Serial.print("Setpoint:");  Serial.print(Setpoint);  Serial.print(",");
  Serial.print("Input:");     Serial.print(Input);     Serial.print(",");
  Serial.print("Output:");    Serial.print(Output);    Serial.print(",");
  Serial.println(",");

  Input = myQuickPID.analogReadFast(inputPin);
  myQuickPID.Compute();
  analogWrite(outputPin, Output);
}