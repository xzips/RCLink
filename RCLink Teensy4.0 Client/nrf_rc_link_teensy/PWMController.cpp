

#include "PWMController.hpp"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "Arduino.h"
#include "RadioController.hpp"

Adafruit_PWMServoDriver pwm_driver = Adafruit_PWMServoDriver();

unsigned long last_smooth_pwm_call;

#define SMOOTH_PWM_UPDATE_DELAY_MS 50




namespace pwm {
  


  unsigned long last_pwm_update_millis;
  std::vector<SmoothPWM> smooth_pwms;



  bool should_update_pwm()
  {
    unsigned long current_millis = millis();
    unsigned long elapsed_millis = current_millis - last_pwm_update_millis;



    if (elapsed_millis >= SMOOTH_PWM_UPDATE_DELAY_MS)
    {
      last_smooth_pwm_call = current_millis;
      return true;


    }

    return false;

  }


  void setup_pwm() {

    pwm_driver.begin();
    pwm_driver.setOscillatorFrequency(27000000);
    pwm_driver.setPWMFreq(SERVO_FREQ);

    last_pwm_update_millis = millis();


    delay(10);
  }

  int angle2pulse_MS24(int angleDegrees)
  {
    //if (angleDegrees < 0) angleDegrees = 0;
    //else if (angleDegrees > 270) angleDegrees = 270;
   

    float m = (float)(PULSE_270 - PULSE_0) / (float)(270 - 0);
    float b = (float)PULSE_0;
    float pulse = m * angleDegrees + b;

    return (int)pulse;
  }


   

  void setThrottle(int pulseWidth) {
    int onTick = 0; // Always start pulse at the beginning of the cycle
    int offTick = map(pulseWidth, 1500, 2000, 210 , 410); // Correct mapping based on 4096 ticks
    pwm_driver.setPWM(ESC_CHANNEL, onTick, offTick); // Assuming we're using channel 0 for the ESC
  }

  void set_servo_angle(uint8_t servoNum, int angleDegrees)
  {
    int anglePulse = angle2pulse_MS24(angleDegrees);

    //Serial.println("Setting Servo " + String(servoNum) + " to " + String(angleDegrees) + " degrees, with pulse " + String(anglePulse));

    pwm_driver.setPWM(servoNum, 0, anglePulse);
  }
void update_smooth_pwms()
{
    unsigned long current_millis = millis();
    unsigned long elapsed_millis = current_millis - last_pwm_update_millis;

    if (elapsed_millis < SMOOTH_PWM_UPDATE_DELAY_MS) return;

    last_pwm_update_millis = current_millis;

    float elapsed_seconds = elapsed_millis / 1000.0;

    for (auto &smooth_pwm : smooth_pwms)
    {
        // Skip if already at target and speed is zero
        if (smooth_pwm.current_angle == smooth_pwm.target_angle && smooth_pwm.current_speed == 0)
            continue;

        // Compute angle difference
        float angle_diff = smooth_pwm.target_angle - smooth_pwm.current_angle;

        // Determine desired acceleration
        float desired_acceleration = (angle_diff > 0 ? 1 : -1) * smooth_pwm.max_acceleration;

        // Adjust acceleration if overshooting
        if ((smooth_pwm.current_speed * angle_diff) > 0)
        {
            // Decelerate to stop at the target
            float required_deceleration = - (smooth_pwm.current_speed * smooth_pwm.current_speed) / (2 * angle_diff);
            if (abs(required_deceleration) < abs(smooth_pwm.max_acceleration))
                desired_acceleration = required_deceleration;
        }

        // Update speed
        smooth_pwm.current_speed += desired_acceleration * elapsed_seconds;

        // Limit speed to max_speed
        if (smooth_pwm.current_speed > smooth_pwm.max_speed)
            smooth_pwm.current_speed = smooth_pwm.max_speed;
        else if (smooth_pwm.current_speed < -smooth_pwm.max_speed)
            smooth_pwm.current_speed = -smooth_pwm.max_speed;

        // Update position
        smooth_pwm.current_angle += smooth_pwm.current_speed * elapsed_seconds;

        // Check if we have reached or passed the target position
        if ((smooth_pwm.current_speed > 0 && smooth_pwm.current_angle >= smooth_pwm.target_angle) ||
            (smooth_pwm.current_speed < 0 && smooth_pwm.current_angle <= smooth_pwm.target_angle))
        {
            // Set to target position and zero speed
            smooth_pwm.current_angle = smooth_pwm.target_angle;
            smooth_pwm.current_speed = 0;
        }

        // Send PWM signal
        if (smooth_pwm.is_esc)
        {
            setThrottle((int)smooth_pwm.current_angle);
        }
        else
        {
            set_servo_angle(smooth_pwm.servo_num, (int)smooth_pwm.current_angle);
        }
    }
}


  void add_smooth_pwm(uint8_t servo_num, int default_angle, int target_angle, int max_speed, bool is_esc)
  {
    SmoothPWM smooth_pwm;
    smooth_pwm.servo_num = servo_num;
    smooth_pwm.current_angle = default_angle;
    smooth_pwm.target_angle = target_angle;
    smooth_pwm.max_speed = max_speed;
    smooth_pwm.is_esc = is_esc;

    smooth_pwms.push_back(smooth_pwm);
  }
  
  void update_smooth_pwm_target(uint8_t servo_num, int target_angle)
  {
    for (auto &smooth_pwm : smooth_pwms)
    {
      if (smooth_pwm.servo_num == servo_num)
      {
        smooth_pwm.target_angle = target_angle;
        return;
      }
    }


    if (SERIAL_DEBUG)
    {
      Serial.println("Servo " + String(servo_num) + " not found in smooth_pwms");
    }

  }

}

