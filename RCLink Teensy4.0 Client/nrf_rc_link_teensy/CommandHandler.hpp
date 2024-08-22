#pragma once

#include "StateSync.hpp"

// Define servo channels (update these according to your hardware setup)
#define SERVO_LEFT_AILERON    1
#define SERVO_RIGHT_AILERON   2
#define SERVO_FRONT_WHEEL     5
#define SERVO_LEFT_ELEVATOR   8
#define SERVO_RIGHT_ELEVATOR  10
#define SERVO_RUDDER          9
#define ESC_CHANNEL           4  // Assuming the ESC is channel 6 as per previous example


namespace rcon
{
    void HandleCommand(char* command);
}

