#include <CheapStepper.h>
#include "smoothJoystick.h";

#define joystickXPin A0
#define joystickYPin A1

CheapStepper rotation(8, 9, 10, 11);
CheapStepper pitch(4, 5, 6, 7);

joystick joy(joystickXPin, joystickYPin, 512, 512);

void setup() {
    rotation.setRpm(15);
    pitch.setRpm(15);
    rotation.moveToDegree(true, 0);
    pitch.moveToDegree(true, 0);
    Serial.begin(9600);
}

void loop() {
    joy.update();
    Serial.print("Joy X: ");
    Serial.println(joy.x);
    Serial.print("Joy Y: ");
    Serial.println(joy.y);
    updateStepper(joy.x, rotation);
    updateStepper(joy.y, pitch);
    delay(1);
}

void updateStepper(int position, CheapStepper sm) {
    int speed;
    bool clockWise;
    if (position > 562) {
        speed = map(position, 562, 1023, 10, 22);
        clockWise = true;
    } else if (position < 462) {
        speed = map(position, 0, 462, 22, 10);
        clockWise = false;
    } else {
        sm.stop();
    }

    sm.setRpm(speed);
    sm.step(clockWise);
}

