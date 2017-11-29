#include "smoothJoystick.h"
#include <Button.h>
#include <LiquidCrystal.h>
#include <AccelStepper.h>

#define joystickXPin    A0
#define joystickYPin    A1
#define joystickXCenter 513
#define joystickYCenter 500
#define joystickXThresh 10
#define joystickYThresh 20

#define RPM             1000
#define Step            1
#define pitchDirPin     8
#define pitchStepPin    9
#define rotationDirPin  6
#define rotationStepPin 7

#define rs              12
#define en              11
#define d4              5
#define d5              4
#define d6              3
#define d7              2

#define pushButton      10
#define fireButton      13

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
AccelStepper rotation(1, rotationStepPin, rotationDirPin);
AccelStepper pitch(1, pitchStepPin, pitchDirPin);
Button btn(pushButton, false, false, 20);

joystick joy(
    joystickXPin,
    joystickYPin,
    joystickXCenter,
    joystickYCenter
);

char buf[20];
int autoX = 0;
int autoY = 0;
bool manual = true;
bool longPress = false;

void setup() {
    lcd.begin(16, 2);
    lcd.noCursor();
    rotation.setMaxSpeed(RPM);
    pitch.setMaxSpeed(RPM);
    pinMode(fireButton, OUTPUT);
    digitalWrite(fireButton, LOW);
    Serial.begin(115200);
}

void loop() {
    joy.update();
    doButtonActions();
    if (manual) {
        updateStepper(joy.x, joystickXThresh, rotation);
        updateStepper(joy.y, joystickYThresh, pitch);   
    } else {
//        updateStepper(autoX, joystickXThresh, rotation);
//        updateStepper(autoY, joystickYThresh, pitch); 
    }
}

void doButtonActions() {
    btn.read();
    if (btn.pressedFor(3000) && !longPress) {
        lcd.clear();
        if (manual) {
            String lcdStr1 = "Release for";
            String lcdStr2 = "Auto Mode";
            printToLCD(lcdStr1, lcdStr2);
        } else {
            String lcdStr1 = "Release for";
            String lcdStr2 = "Manual Mode";
            printToLCD(lcdStr1, lcdStr2);
        }
        longPress = true;
    }
    if (btn.wasReleased() && longPress) {
        manual = !manual;
        lcd.clear();
        if (!manual) {
            String lcdStr = "Automatic Mode";
            lcd.setCursor(0, 0);
            lcd.print(lcdStr);
            lcd.setCursor(0, 0);
            delay(5000);
        }
        longPress = false;
        lcd.clear();
    } else if (btn.wasReleased() && !longPress && manual) {
        digitalWrite(fireButton, HIGH);
        delay(10);
        digitalWrite(fireButton, LOW);
    }
    if (manual && !longPress) {
        String lcdStr = "Manual Mode    ";
        lcd.setCursor(0, 0);
        lcd.print(lcdStr);
        lcd.setCursor(0, 0);
    }
    if (!manual && !longPress) {
        updateFromSerial();
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
    printToLCD(xStr, yStr);
}

void updateStepper(int position, int thresh, AccelStepper sm) {
    int jsIn = map(position, 0, 1023, -512, 512);
    if (abs(jsIn) < thresh) {
        jsIn = 0;
    }
    if (jsIn < 0 && sm.distanceToGo() == 0) { 
      sm.move(-Step);
    } else if (jsIn > 0 && sm.distanceToGo() == 0) { 
      sm.move(Step); 
    } else {
      sm.stop();
    }
    sm.runToPosition();
}

void printToLCD(String first, String second) {
    lcd.setCursor(0, 0);
    lcd.print(first);
    lcd.setCursor(0, 1);
    lcd.print(second);
}

