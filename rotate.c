#include <wiringPi.h>
#include <iostream>
#include <unistd.h>

#define EN_PIN 24
#define STEP_PIN 18
#define DIR_PIN 23


void moveMotor(int steps, int direction, float del) {
    digitalWrite(DIR_PIN, direction);
    
    for (int i = 0; i < steps; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(del);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(del);
    }
    
}

int main() {
    if (wiringPiSetupGpio() == -1) {
        std::cout << "Failed to initialize WiringPi" << std::endl;
        return 1;
    }
    pinMode(EN_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
	pullUpDnControl(EN_PIN, PUD_UP);

	float ms=32.f;
	float base_delay_micro=500.f;
	float full=200.f;
	float slow=4.f/12.f;
	float fast=8.f/12.f;

    digitalWrite(EN_PIN, LOW);  // Enable the motor

    // Rotate counterclockwise for 200 steps
	for(int i=0; i<5; ++i){
//		moveMotor(full * fast * ms, HIGH, base_delay_micro / 4.f / 2.f);
		moveMotor(full * ms, HIGH, base_delay_micro/8.f);
	}
    
    digitalWrite(EN_PIN, HIGH);  // Disable the motor

    return 0;
}
