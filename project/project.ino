#include "BasicStepperDriver.h"
#include "SyncDriver.h"
#include "smoothJoystick.h"
#include <Button.h>
#include <LiquidCrystal.h>

#define joystickXPin    A0
#define joystickYPin    A1
#define joystickEnable  2
#define joystickXCenter 513
#define joystickYCenter 500
#define joystickXThresh 10
#define joystickYThresh 20

#define MotorSteps      400
#define RPM             50
#define MicroSteps      2
#define MaxRPM          50
#define Step            2
#define pitchDirPin     8
#define pitchStepPin    9
#define rotationDirPin  6
#define rotationStepPin 7

#define rs              7
#define en              6
#define d4              5
#define d5              4
#define d6              3
#define d7              2

#define laserEnable     7
#define pushButton      10

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
BasicStepperDriver rotation(MotorSteps, rotationDirPin, rotationStepPin);
BasicStepperDriver pitch(MotorSteps, pitchDirPin, pitchStepPin);
Button btn(pushButton, false, false, 20);

joystick joy(
    joystickEnable,
    joystickXPin,
    joystickYPin,
    joystickXCenter,
    joystickYCenter
);

char buf[20];
int autoX = 0;
int autoY = 0;
bool manual = true;

void setup() {
    lcd.begin(16, 2);
    lcd.noCursor();
    rotation.begin(RPM, MicroSteps);
    pitch.begin(RPM, MicroSteps);
    pinMode(laserEnable, OUTPUT);
    digitalWrite(laserEnable, LOW);
    Serial.begin(115200);
}

void loop() {
    joy.update();
    updateFromSerial();
    if (manual) {
        updateStepper(joy.x, joystickXThresh, rotation);
        updateStepper(joy.y, joystickYThresh, pitch);   
    } else {
        updateStepper(autoX, joystickXThresh, rotation);
        updateStepper(autoY, joystickYThresh, pitch); 
    }
    if (btn.pressedFor(3000)) {
        Serial.println("Button has been pressed for 3 secs");
    }
}

void updateFromSerial() {
    for (int i = 0; Serial.available() > 0; i++) {
        buf[i] = (char)Serial.read();
    }
    sscanf(buf, "%d,%d", &autoX, &autoY);
    memset(&buf, 0, 20);
    String xStr = "X: ";
    xStr.concat(autoX);
    xStr.concat("   ");
    String yStr = "Y: ";
    yStr.concat(autoY);
    yStr.concat("   ");
    lcd.setCursor(0, 0);
    lcd.print(xStr);
    lcd.setCursor(0, 1);
    lcd.print(yStr);
}

void updateStepper(int position, int thresh, BasicStepperDriver sm) {
    int jsIn = map(position, 0, 1023, -512, 512);
    if (abs(jsIn) < thresh) {
        jsIn = 0;
    }
    int finalRPM = map(jsIn, -512, 512, -MaxRPM, MaxRPM);
    sm.setRPM(abs(finalRPM));
    delay(1);
    if (finalRPM < 0) { 
      sm.move(-Step);
    } else if (finalRPM > 0) { 
      sm.move(Step); 
    } else {
      sm.stop();
    }
}

