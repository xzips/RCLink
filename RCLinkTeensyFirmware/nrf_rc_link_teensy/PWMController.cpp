/*

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVO_FREQ 50  // Analog servos run at ~50 Hz updates

// Define the pulse lengths corresponding to each position
#define PULSE_0    123   // 500us
#define PULSE_45   205   // 833us
#define PULSE_90   287   // 1166us
#define PULSE_135  369   // 1500us
#define PULSE_180  451   // 1833us
#define PULSE_225  533   // 2167us
#define PULSE_270  615   // 2500us

void setup() {
  Serial.begin(9600);
  Serial.println("Servo test!");

  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

  delay(10);
}

void loop() {
  uint8_t servonum = 15;

  // Move to 0°
  pwm.setPWM(servonum, 0, PULSE_0);
  delay(1000);

  // Move to 45°
  pwm.setPWM(servonum, 0, PULSE_45);
  delay(1000);

  // Move to 90°
  pwm.setPWM(servonum, 0, PULSE_90);
  delay(1000);

  // Move to 135° (Neutral Position)
  pwm.setPWM(servonum, 0, PULSE_135);
  delay(1000);

  // Move to 180°
  pwm.setPWM(servonum, 0, PULSE_180);
  delay(1000);

  // Move to 225°
  pwm.setPWM(servonum, 0, PULSE_225);
  delay(1000);

  // Move to 270°
  pwm.setPWM(servonum, 0, PULSE_270);
  delay(1000);

  // Move back to 225°
  pwm.setPWM(servonum, 0, PULSE_225);
  delay(1000);

  // Move back to 180°
  pwm.setPWM(servonum, 0, PULSE_180);
  delay(1000);

  // Move back to 135° (Neutral Position)
  pwm.setPWM(servonum, 0, PULSE_135);
  delay(1000);

  // Move back to 90°
  pwm.setPWM(servonum, 0, PULSE_90);
  delay(1000);

  // Move back to 45°
  pwm.setPWM(servonum, 0, PULSE_45);
  delay(1000);

  // Move back to 0°
  pwm.setPWM(servonum, 0, PULSE_0);
  delay(1000);
}


*/