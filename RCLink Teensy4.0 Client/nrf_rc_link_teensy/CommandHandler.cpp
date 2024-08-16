#include "CommandHandler.hpp"
#include <Arduino.h>
#include "PWMController.hpp"
#include "IMUController.hpp"
#include <string>
#include "RadioController.hpp"

namespace rcon
{

    void doReboot() {
        SCB_AIRCR = 0x05FA0004;
    }

    void HandleCommand(char* command)
    {
        Serial.println(command);
        
        //static const char* 


        /* MULTI SERVO CONTROL*/
        //command looks like: MS4NNXXX-MMXXX-LLXXX-KKXXX-PPXXX
        //MS401135-02135-05090-08025
       // MS4NNXXX-MMXXX-LLXXX-KKXXX-PPXXX
        //i.e. servo number and then angle, separated by a dash, always exactly 4 servos

        static const char* multi_servo_str = "S5";

        if (strncmp(command, multi_servo_str, strlen(multi_servo_str)) == 0)
        {

            std::string command_str(command);

            //Serial.println(command_str.c_str());

            if (command_str.length() < 31)
            {
                return;
            }

         
            for (int i = 0; i < 5; i++)
            {

                int physical_servo_num = atoi (command_str.substr(2 + i * 6, 2).c_str());

                if (physical_servo_num == 99)
                {
                  continue;
                }

                int angle = atoi(command_str.substr(4 + i * 6, 3).c_str());


                if (SERIAL_DEBUG)
                {
                    //Serial.println("Multi Set Servo Did: Servo: " + String(physical_servo_num) + " Angle: " + String(angle));
                }



               

                pwm::update_smooth_pwm_target(physical_servo_num, angle);
            }

        }

        
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

        /* THROTTLE CONTROL, "SET_THROTTLE_XXXX" where XXXX is the pulse width */
        static const char* set_throttle_str = "SET_THROTTLE_";
        if (strncmp(command, set_throttle_str, strlen(set_throttle_str)) == 0)
        {
            char* throttle_str = command + strlen(set_throttle_str);
            int throttle = atoi(throttle_str);

           // pwm::setThrottle(throttle);

            pwm::update_smooth_pwm_target(ESC_CHANNEL, throttle);

            //print throttle just set
           // Serial.println("Throttle: " + String(throttle));

        }

        /* PWM RAW CONTROL, SET_PWM_XX_XXXX_XXXX, assume order of channel, onTick, offTick from 0 to 4095 */
        static const char* set_pwm_str = "SET_PWM_";
        if (strncmp(command, set_pwm_str, strlen(set_pwm_str)) == 0)
        {
            char* pwm_str = command + strlen(set_pwm_str);
            char* channel_str = pwm_str;
            char* on_tick_str = channel_str + 3;
            char* off_tick_str = on_tick_str + 5;
            int channel = atoi(channel_str);
            int on_tick = atoi(on_tick_str);
            int off_tick = atoi(off_tick_str);

            pwm_driver.setPWM(channel, on_tick, off_tick);

            //Serial.println("zeroing pwm!!");

            //print all values we just set
            //Serial.println("Channel: " + String(channel) + " OnTick: " + String(on_tick) + " OffTick: " + String(off_tick));

        }


        /*IMU_CALIBRATE*/
        static const char* calibrate_str = "REBOOT";
        if (strncmp(command, calibrate_str, strlen(calibrate_str)) == 0)
        {
            doReboot();

        }



        


        

    }
}
