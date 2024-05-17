

#include "PWMController.hpp"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "Arduino.h"

Adafruit_PWMServoDriver pwm_driver = Adafruit_PWMServoDriver();

namespace pwm {
  void setup_pwm() {

    pwm_driver.begin();
    pwm_driver.setOscillatorFrequency(27000000);
    pwm_driver.setPWMFreq(SERVO_FREQ);


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

  void angle2pulse_ESC(int dutyCycle)
  {
    
  }

  void set_servo_angle(uint8_t servoNum, int angleDegrees)
  {
    int anglePulse = angle2pulse_MS24(angleDegrees);

    //Serial.println("Setting Servo " + String(servoNum) + " to " + String(angleDegrees) + " degrees, with pulse " + String(anglePulse));

    pwm_driver.setPWM(servoNum, 0, anglePulse);
  }


}

