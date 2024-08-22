#include "CommandHandler.hpp"
#include <Arduino.h>
#include "PWMController.hpp"
#include "StateSync.hpp"
#include <string.h>

namespace rcon
{
    void doReboot() {
        SCB_AIRCR = 0x05FA0004;
    }

    void HandleCommand(char* command)
    {
        // Assume the command is a base64 encoded ControllerState
        ControllerState receivedState;

        //strip any non-base64 characters such as \n
        command[strcspn(command, "\n")] = 0;

        //if the command is all AAA ignore it

        bool isAllA = true;
        for (int i = 0; i < strlen(command); i++)
        {
            if (command[i] != 'A')
            {
                isAllA = false;
                break;
            }
        }

        if (isAllA)
        {
            return;
        }


        // Decode the base64 command into the ControllerState struct
        if (!decode_ControllerState(command, &receivedState)) {
            // Handle error: Invalid command format or decoding failure


            Serial.println("Invalid command format or decoding failure");
            


            return;
        }

        

        // Update PWM outputs based on the received state
        pwm::update_smooth_pwm_target(SERVO_LEFT_AILERON, receivedState.LeftAileron);
        pwm::update_smooth_pwm_target(SERVO_RIGHT_AILERON, receivedState.RightAileron);
        pwm::update_smooth_pwm_target(SERVO_FRONT_WHEEL, receivedState.FrontWheel);
        pwm::update_smooth_pwm_target(SERVO_LEFT_ELEVATOR, receivedState.LeftElevator);
        pwm::update_smooth_pwm_target(SERVO_RIGHT_ELEVATOR, receivedState.RightElevator);
        pwm::update_smooth_pwm_target(SERVO_RUDDER, receivedState.Rudder);
        pwm::update_smooth_pwm_target(ESC_CHANNEL, receivedState.Throttle);

        // Handle MCU reset if requested
        if (receivedState.MCUReset) {
            doReboot();
        }
    }
}

