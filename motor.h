#ifndef MOTOR_H
#define MOTOR_H

#include <pigpio.h>

class Motor {
public:
    Motor(int enPin, int dirPin, int stepPin);
    ~Motor();
    void enable();
    void disable();
    void rotate(int steps, int delayUs, bool direction);

private:
    int enPin_;
    int dirPin_;
    int stepPin_;
    bool isEnabled_;
};

#endif