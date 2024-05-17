
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

extern Adafruit_PWMServoDriver pwm_driver;

#define SERVO_FREQ 50

#define PULSE_0    123   // 500us
#define PULSE_45   205   // 833us
#define PULSE_90   287   // 1166us
#define PULSE_135  369   // 1500us
#define PULSE_180  451   // 1833us
#define PULSE_225  533   // 2167us
#define PULSE_270  615   // 2500us


namespace pwm{
    void setup_pwm();

  int angle2pulse_MS24(int angleDegrees);
  void angle2pulse_ESC(int dutyCycle);
  void set_servo_angle(uint8_t servoNum, int angleDegrees);
}