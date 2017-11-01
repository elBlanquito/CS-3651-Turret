#include "Arduino.h"
#include "smoothJoystick.h"

joystick::joystick(byte analogXPin, byte analogYPin, int initialXVal, int initialYVal)
{

    _analogXPin = analogXPin;
    _analogYPin = analogYPin;
    _initialXVal = initialXVal;
    _initialYVal = initialYVal;

    pinMode(_analogXPin, INPUT);
    pinMode(_analogYPin, INPUT);

    for (int i = 0; i < _size; i++)
    {
        _bufferX[i] = _initialXVal;
        _bufferXSum += _initialXVal;
    }

    for (int i = 0; i < _size; i++)
    {
        _bufferY[i] = _initialYVal;
        _bufferYSum += _initialYVal;
    }

    _bufferIndex = 0;
}

void joystick::update(void)
{

    int readAnalogXVal = analogRead(_analogXPin);

    _bufferXSum = _bufferXSum - _bufferX[_bufferIndex];
    _bufferXSum = _bufferXSum + readAnalogXVal;
    _bufferX[_bufferIndex] = readAnalogXVal;

    x = _bufferXSum >> _shift;

    int readAnalogYVal = analogRead(_analogYPin);

    _bufferYSum = _bufferYSum - _bufferY[_bufferIndex];
    _bufferYSum = _bufferYSum + readAnalogYVal;
    _bufferY[_bufferIndex] = readAnalogYVal;

    y = _bufferYSum >> _shift;

    _bufferIndex = (_bufferIndex + 1) % _size;
}
