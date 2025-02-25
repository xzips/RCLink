#include <Arduino.h>
#include "RF24.h"
#include "RadioController.hpp"
#include "OLEDController.hpp"
#include "PWMController.hpp"
#include "CommandHandler.hpp"
#include "MPU6050_6Axis_MotionApps612.h"
#include "IMUController.hpp"

#include "SPI.h"

unsigned long startTime;
unsigned long endTime;
unsigned long elapsedTime;





void setup() {

 // Wire.setClock(400000);

  Serial.begin(9600);
  if (CrashReport) {
      /* print info (hope Serial Monitor windows is open) */
      Serial.print(CrashReport);
    }


  rcon::radio_setup();
  disp::setup_display();
  pwm::setup_pwm();
  imu::imu_setup();

  last_packet_timestamp_millis = millis();

    
  const int MS24_SPEED_DEG_PER_SEC = 500;


  
  pwm::add_smooth_pwm(1, 135, 135, MS24_SPEED_DEG_PER_SEC);
  pwm::add_smooth_pwm(2, 135, 135, MS24_SPEED_DEG_PER_SEC);

  pwm::add_smooth_pwm(5, 90, 90, MS24_SPEED_DEG_PER_SEC);

  pwm::add_smooth_pwm(8, 95, 95, MS24_SPEED_DEG_PER_SEC);

  pwm::add_smooth_pwm(9, 90, 90, MS24_SPEED_DEG_PER_SEC);

  pwm::add_smooth_pwm(10, 95, 95, MS24_SPEED_DEG_PER_SEC);

  pwm::add_smooth_pwm(ESC_CHANNEL, 1500, 1500, 500, true);
  

  



}  




void loop() {


  startTime = micros();






  //currently blocking!!
  rcon::RadioLoopState state = rcon::radio_loop();

  //pull pin 12 high
  digitalWrite(12, HIGH);

  endTime = micros();
  elapsedTime = endTime - startTime;
  
  //Serial.println("RF24 radio_loop function: " + String(elapsedTime) + " microseconds");

  
  imu::update_rotation_ypr();
  imu::UpdateYPRTelemetry();



  if (state != rcon::RadioLoopState::CONNECTED_IDLE) {
    //rcon::print_radio_loop_state(state);

  }



  if (!incomingBufferProcessed)
  {
    rcon::HandleCommand(rf_incoming_buffer);
    incomingBufferProcessed = true;
  }
  
  
  if (pwm::should_update_pwm())
  {
    pwm::update_smooth_pwms();
  }
  




  if (disp::should_update_display()) {
    disp::update_stats_V1(radioConnected, millis() - last_packet_timestamp_millis, millis(), rf_incoming_buffer);
  }
 


} 
