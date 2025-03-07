#pragma once
#include <osc++.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "glob.h"
#include "motor.h"
#include "receiver.h"

