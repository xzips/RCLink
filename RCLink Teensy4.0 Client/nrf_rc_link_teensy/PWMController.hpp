
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <vector>

extern Adafruit_PWMServoDriver pwm_driver;

#define SERVO_FREQ 50

#define PULSE_0    123   // 500us
#define PULSE_45   205   // 833us
#define PULSE_90   287   // 1166us
#define PULSE_135  369   // 1500us
#define PULSE_180  451   // 1833us
#define PULSE_225  533   // 2167us
#define PULSE_270  615   // 2500us


const int PULSE_FULL_REVERSE = 205;  // Approx 1000 microseconds
const int PULSE_FULL_THROTTLE = 410;  // Approx 2000 microseconds
const int PULSE_NEUTRAL = 307;        // Approx 1500 microseconds



#define SMOOTH_PWM_UPDATE_DELAY_MS 50

#define ESC_CHANNEL 4


namespace pwm{
  void setup_pwm();

  bool should_update_pwm();

  int angle2pulse_MS24(int angleDegrees);

  void set_servo_angle(uint8_t servoNum, int angleDegrees);

  void setThrottle(int pulseWidth);

  void update_smooth_pwms();
  void add_smooth_pwm(uint8_t servo_num, int default_angle, int target_angle, int max_speed, bool is_esc = false);
  void update_smooth_pwm_target(uint8_t servo_num, int target_angle);

  struct SmoothPWM {
    uint8_t servo_num;
    float current_angle;
    float target_angle;
    float max_speed; // degrees per second
    bool is_esc;
  };

  extern unsigned long last_pwm_update_millis;

  extern std::vector<SmoothPWM> smooth_pwms;





}