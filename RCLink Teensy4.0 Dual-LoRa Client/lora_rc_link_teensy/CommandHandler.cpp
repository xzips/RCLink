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
        for (size_t i = 0; i < strlen(command); i++)
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
        if (!decode(command, receivedState)) {
            // Handle error: Invalid command format or decoding failure


            Serial.print("Invalid command format or decoding failure: ");
            Serial.println(command);


            return;
        }

        if (receivedState.NetworkID == 0xA8)
        {
            controllerState = receivedState;
        }

        else
        {
            Serial.println("Network ID mismatch, ignoring packet");
            Serial.print("Received Network ID: ");
            Serial.println(receivedState.NetworkID);
            return;
        }

        
        print_struct(controllerState);
          

        // Update PWM outputs based on the received state
        pwm::update_smooth_pwm_target(SERVO_LEFT_AILERON, (receivedState.LeftAileron / 255.f) * 360.f);
        pwm::update_smooth_pwm_target(SERVO_RIGHT_AILERON, (receivedState.RightAileron / 255.f) * 360.f);
        pwm::update_smooth_pwm_target(SERVO_FRONT_WHEEL, (receivedState.FrontWheel / 255.f) * 360.f);
        pwm::update_smooth_pwm_target(SERVO_LEFT_ELEVATOR, (receivedState.LeftElevator / 255.f) * 360.f);
        pwm::update_smooth_pwm_target(SERVO_RIGHT_ELEVATOR, (receivedState.RightElevator / 255.f) * 360.f);
        pwm::update_smooth_pwm_target(SERVO_RUDDER, (receivedState.Rudder / 255.f) * 360.f);

        // Inverse the throttle compression
        float throttleValue = ((receivedState.Throttle / 255.f) * 500.f) + 1500.f;
        pwm::update_smooth_pwm_target(ESC_CHANNEL, throttleValue);

        // Handle MCU reset if requested
        if (receivedState.MCUReset) {
            Serial.println("REBOOT WAS REQUESTED BY REMOTE CONTROL, REBOOTING!!!");
            doReboot();
        }



    }
}

