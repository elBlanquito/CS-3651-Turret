#include "Arduino.h"
#include "smoothJoystick.h"

joystick::joystick(
    byte enablePin, 
    byte analogXPin, 
    byte analogYPin, int initialXVal, int initialYVal) {

    _analogXPin = analogXPin;
    _analogYPin = analogYPin;
    _enablePin = enablePin;
    _initialXVal = initialXVal;
    _initialYVal = initialYVal;

    pinMode(_analogXPin, INPUT);
    pinMode(_analogYPin, INPUT);
    pinMode(_enablePin, OUTPUT);
    digitalWrite(_enablePin, HIGH);

    for (int i = 0; i < _size; i++) {
        _bufferX[i] = _initialXVal;
        _bufferXSum += _initialXVal;
        _bufferY[i] = initialYVal;
        _bufferYSum += _initialYVal;
    }

    _bufferIndex = 0;
}

void joystick::update(void)
{
    int readAnalogXVal = analogRead(_analogXPin);

    _bufferXSum = _bufferXSum - _bufferX[_bufferIndex];
    _bufferXSum += readAnalogXVal;
    _bufferX[_bufferIndex] = readAnalogXVal;

    x = _bufferXSum >> _shift;

    int readAnalogYVal = analogRead(_analogYPin);

    _bufferYSum = _bufferYSum - _bufferY[_bufferIndex];
    _bufferYSum += readAnalogYVal;
    _bufferY[_bufferIndex] = readAnalogYVal;

    y = _bufferYSum >> _shift;

    _bufferIndex = (_bufferIndex + 1) % _size;
}
