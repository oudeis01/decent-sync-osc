#include "motor.h"
#include "receiver.h"
#include <pigpio.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

#define EN_PIN 24
#define DIR_PIN 23
#define STEP_PIN 18