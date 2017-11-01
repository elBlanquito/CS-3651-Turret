#ifndef _smoothJoystick_h_
#define _smoothJoystick_h_

#include "Arduino.h";

class joystick {
  public:
    joystick(byte analogXPin, byte analogYPin, int initialXVal, int initialYVal);
    void joystick::update(void);
    int x, y;
  private:
    static const int _size = 64;
    static const int _shift = 6;
    int _bufferX[_size];
    int _bufferY[_size];
    byte _analogXPin;
    byte _analogYPin;
    long _bufferXSum;
    long _bufferYSum;
    int _bufferIndex;
    int _bufferSize;
    int _initialXVal;
    int _initialYVal;
};


#endif
