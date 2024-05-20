#include "CommandHandler.hpp"
#include <Arduino.h>
#include "PWMController.hpp"
//#include <cstring>

namespace rcon
{
    void HandleCommand(char* command)
    {
        
        
        /* SERVO CONTROL */
        static const char* set_servo_str = "SET_SERVO_";
        if (strncmp(command, set_servo_str, strlen(set_servo_str)) == 0)
        {
            char* servo_num_str = command + strlen(set_servo_str);
            char* angle_str = servo_num_str + 3;
            int servo_num = atoi(servo_num_str);
            int angle = atoi(angle_str);

            //pwm::set_servo_angle(servo_num, angle);

            pwm::update_smooth_pwm_target(servo_num, angle);

        }
        


        

    }
}
