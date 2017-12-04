#include "smoothJoystick.h"
#include <Button.h>
#include <LiquidCrystal.h>
#include <AccelStepper.h>

// Joystick
#define joystickXPin    A0
#define joystickYPin    A1
#define joystickXCenter 513
#define joystickYCenter 500
#define joystickXThresh 10
#define joystickYThresh 20

// Stepper Motors
#define RPM             1000
#define Step            1
#define pitchDirPin     8
#define pitchStepPin    9
#define rotationDirPin  6
#define rotationStepPin 7

// LCD Screen
#define rs              12
#define en              11
#define d4              5
#define d5              4
#define d6              3
#define d7              2

// Button & Trigger Relay
#define pushButton      10
#define triggerRelay    13

// Other
#define buffSize        20
#define triggerDelay    10

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

char buf[buffSize];
int autoX = joystickXCenter;
int autoY = joystickYCenter;
bool manual = true;
bool longPress = false;
bool rpiCom = false;

void setup() {
    lcd.begin(16, 2);
    lcd.noCursor();
    rotation.setMaxSpeed(RPM);
    pitch.setMaxSpeed(RPM);
    pinMode(triggerRelay, OUTPUT);
    digitalWrite(triggerRelay, LOW);
    Serial.begin(115200);
    Serial.println("manual");
    Serial.flush();
}

void loop() {
    joy.update();
    doButtonActions();
    if (manual) {
       updateStepper(joy.x, joystickXThresh, rotation);
       updateStepper(joy.y, joystickYThresh, pitch);
    } else if (!manual && rpiCom) {
       updateStepper(autoX, joystickXThresh, rotation);
       updateStepper(autoY, joystickYThresh, pitch);
    }
}

void doButtonActions() {
    btn.read();
    if (btn.pressedFor(3000) && !longPress) {
        if (manual) {
            String lcdStr1 = "Release for";
            String lcdStr2 = "Auto Mode";
            printToLCD2(lcdStr1, lcdStr2);
        } else {
            String lcdStr1 = "Release for";
            String lcdStr2 = "Manual Mode";
            autoX = joystickXCenter;
            autoY = joystickYCenter;
            printToLCD2(lcdStr1, lcdStr2);
        }
        longPress = true;
    }
    if (btn.wasReleased() && longPress) {
        manual = !manual;
        if (!manual) {
            String lcdStr = "Automatic Mode";
            printToLCD(lcdStr);
            delay(5000);
            Serial.println("auto");
            Serial.flush();
        }
        if (manual) {
            Serial.println("manual");
            Serial.flush();
        }
        longPress = false;
    } else if (btn.wasReleased() && !longPress && manual) {
        digitalWrite(triggerRelay, HIGH);
        delay(triggerDelay);
        digitalWrite(triggerRelay, LOW);
    }
    if (manual && !longPress) {
        String lcdStr = "Manual Mode";
        printToLCD(lcdStr);
    } else if (!manual && !longPress) {
        updateFromSerial();
    }
}

void updateFromSerial() {
    if (Serial.available() > 0) {
        rpiCom = true;
    }
    for (int i = 0; Serial.available() > 0 && i < buffSize; i++) {
        buf[i] = (char)Serial.read();
    }
    if (rpiCom) {
        sscanf(buf, "%d,%d", &autoX, &autoY);
        memset(&buf, 0, buffSize);
        String xStr = "X: ";
        xStr.concat(autoX);
        String yStr = "Y: ";
        yStr.concat(autoY);
        printToLCD2(xStr, yStr);
    } else {
        String lcdStr1 = "Waiting On RPI3";
        printToLCD(lcdStr1);
    }
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

void printToLCD2(String first, String second) {
    while (first.length() < 16) first.concat(" ");
    while (second.length() < 16) second.concat(" ");
    lcd.setCursor(0, 0);
    lcd.print(first);
    lcd.setCursor(0, 1);
    lcd.print(second);
}

void printToLCD(String first) {
    while (first.length() < 16) first.concat(" ");
    String second = "                ";
    lcd.setCursor(0, 0);
    lcd.print(first);
    lcd.setCursor(0, 1);
    lcd.print(second);
}

