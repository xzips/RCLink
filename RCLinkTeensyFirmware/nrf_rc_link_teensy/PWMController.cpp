

#include "PWMController.hpp"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "Arduino.h"
#include "RadioController.hpp"

Adafruit_PWMServoDriver pwm_driver = Adafruit_PWMServoDriver();



namespace pwm {

  unsigned long last_pwm_update_millis;
  std::vector<SmoothPWM> smooth_pwms;

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

  
  int throttle2pulse_ESC(float throttle) {
    // Constrain the throttle value to ensure it's between -1.0 (full reverse) and 1.0 (full throttle)
    throttle = constrain(throttle, -1.0, 1.0);
    
    int pulseWidth;
    if (throttle >= 0) {
      // Map the throttle from 0 to 1.0 to neutral to full throttle pulse width
      pulseWidth = map(throttle * 100, 0, 100, PULSE_NEUTRAL, PULSE_FULL_THROTTLE);
    } else {
      // Map the throttle from -1.0 to 0 to full reverse to neutral pulse width
      pulseWidth = map(throttle * 100, -100, 0, PULSE_FULL_REVERSE, PULSE_NEUTRAL);
    }
    
    return pulseWidth;
  }

    void set_esc_throttle(float throttle)
    {
    int pulseWidth = throttle2pulse_ESC(throttle);
    Serial.print("Setting ESC throttle to ");
    Serial.print(throttle, 3);
    Serial.print(", Pulse Width: ");
    Serial.println(pulseWidth);

    // Assuming the ESC is connected to slot 4 of the PCA9685
    pwm_driver.setPWM(4, 0, pulseWidth);
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

    //print in func
   // Serial.println("Elapsed millis: " + String(elapsed_millis));

    

    if (elapsed_millis < SMOOTH_PWM_UPDATE_DELAY_MS) return;

    last_pwm_update_millis = current_millis;

    for (auto &smooth_pwm : smooth_pwms)
    {
      if (smooth_pwm.current_angle == smooth_pwm.target_angle) continue;

      float angle_diff = smooth_pwm.target_angle - smooth_pwm.current_angle;
      float angle_diff_abs = abs(angle_diff);

      int angle_diff_sign = angle_diff > 0 ? 1 : -1;

      float angle_diff_step = smooth_pwm.max_speed * (elapsed_millis / 1000.0);

      //print all vars
      //Serial.println("Servo " + String(smooth_pwm.servo_num) + " angle_diff: " + String(angle_diff));
      //Serial.println("Servo " + String(smooth_pwm.servo_num) + " angle_diff_abs: " + String(angle_diff_abs));
      //Serial.println("Servo " + String(smooth_pwm.servo_num) + " angle_diff_sign: " + String(angle_diff_sign));
      //Serial.println("Servo " + String(smooth_pwm.servo_num) + " angle_diff_step: " + String(angle_diff_step));


      if (angle_diff_abs <= angle_diff_step) {
        smooth_pwm.current_angle = smooth_pwm.target_angle;
      }
      else {
        smooth_pwm.current_angle += angle_diff_sign * angle_diff_step;
      }

      //print current angle

      //Serial.println("Servo " + String(smooth_pwm.servo_num) + " current angle: " + String(smooth_pwm.current_angle));
      //Serial.println("Servo " + String(smooth_pwm.servo_num) + " target angle: " + String(smooth_pwm.target_angle));

      

      set_servo_angle(smooth_pwm.servo_num, (int)smooth_pwm.current_angle);

    }

  }

  void add_smooth_pwm(uint8_t servo_num, int target_angle, int max_speed)
  {
    SmoothPWM smooth_pwm;
    smooth_pwm.servo_num = servo_num;
    smooth_pwm.current_angle = 135;
    smooth_pwm.target_angle = target_angle;
    smooth_pwm.max_speed = max_speed;

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

