#include <Arduino.h>
#include "RF24.h"
#include "RadioController.hpp"
#include "OLEDController.hpp"
#include "PWMController.hpp"
#include "CommandHandler.hpp"
#include "MPU6050_6Axis_MotionApps612.h"
#include "IMUController.hpp"



unsigned long startTime;
unsigned long endTime;
unsigned long elapsedTime;

RF24 radio(CE_PIN, CSN_PIN);




void setup() {
  rcon::radio_setup(radio);
  disp::setup_display();
  pwm::setup_pwm();
  imu::imu_setup();

  last_packet_timestamp_millis = millis();

    
  const int MS24_SPEED_DEG_PER_SEC = 180;
  
  pwm::add_smooth_pwm(15, 135, 135, MS24_SPEED_DEG_PER_SEC);
  pwm::add_smooth_pwm(ESC_CHANNEL, 1500, 1500, 500, true);


  
  

}  




void loop() {




  startTime = micros();

  //currently blocking!!
  rcon::RadioLoopState state = rcon::radio_loop(radio);

  endTime = micros();
  elapsedTime = endTime - startTime;
  
  //Serial.println("RF24 radio_loop function: " + String(elapsedTime) + " microseconds");

  
  imu::update_rotation_ypr();
  imu::time_conditional_send_ypr();

  if (state != rcon::RadioLoopState::CONNECTED_IDLE) {
    //rcon::print_radio_loop_state(state);

  }




  rcon::HandleCommand(rf_incoming_buffer);
  pwm::update_smooth_pwms();




  if (disp::should_update_display()) {
    disp::update_stats_V1(radioConnected, millis() - last_packet_timestamp_millis, millis(), rf_incoming_buffer);
  }
 


} 
