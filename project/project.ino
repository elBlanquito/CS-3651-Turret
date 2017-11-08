#include "BasicStepperDriver.h"
#include "smoothJoystick.h"

#define joystickXPin    A0
#define joystickYPin    A1
#define joystickEnable  2
#define joystickXCenter 513
#define joystickYCenter 500
#define joystickXThresh 10
#define joystickYThresh 20

#define MotorSteps      400
#define RPM             60
#define MicroSteps      2
#define MaxRPM          60
#define Step            6
#define rotationDirPin  9
#define rotationStepPin 10
#define pitchDirPin     4
#define ptichStepPin    5

#define laserEnable     7

BasicStepperDriver rotation(MotorSteps, rotationDirPin, rotationStepPin);
BasicStepperDriver pitch(MotorSteps, pitchDirPin, pitchStepPin);

joystick joy(
    joystickEnable, 
    joystickXPin, 
    joystickYPin, 
    joystickXCenter, 
    joystickYCenter
);

void setup()
{
    rotation.begin(RPM, MicroSteps);
    pinMode(laserEnable, OUTPUT);
    digitalWrite(laserEnable, LOW);
    Serial.begin(115200);
}

void loop()
{
    joy.update();
//    Serial.println(joy.x);
//    Serial.println(joy.y);
    updateStepper(joy.x, joystickXThresh, rotation);
    updateStepper(joy.y, joystickYThresh, pitch);
}

void updateStepper(int position, int thresh, BasicStepperDriver sm) {
    int jsIn = map(position, 0, 1023, -512, 512);
    if (abs(jsIn) < thresh) {
        jsIn = 0;
    }
    int finalRPM = map(jsIn, -512, 512, -MaxRPM, MaxRPM);
    sm.setRPM(abs(finalRPM));
    } if (finalRPM < 0) { 
      sm.move(-Step); 
    } else if (finalRPM > 0) { 
      sm.move(Step); 
    }
}

