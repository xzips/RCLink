#include <Arduino.h>
#include "RF24.h"
#include "RadioController.hpp"
#include "OLEDController.hpp"
#include "PWMController.hpp"
#include "CommandHandler.hpp"

unsigned long startTime;
unsigned long endTime;
unsigned long elapsedTime;

RF24 radio(CE_PIN, CSN_PIN);





void setup() {
  rcon::radio_setup(radio);
  disp::setup_display();
  pwm::setup_pwm();

  last_packet_timestamp_millis = millis();

    
  const int MS24_SPEED_DEG_PER_SEC = 180;
  
  pwm::add_smooth_pwm(15, 135, 135, MS24_SPEED_DEG_PER_SEC);


  pwm::add_smooth_pwm(ESC_CHANNEL, 1500, 1500, 500, true);

/*
  pwm_driver.setPWM(4, 0, 0);
  
  delay(1000);  // Wait for 1 second

  pwm::setThrottle(1500);
  delay(1000);  // Wait for 1 second

  // Set full throttle
  pwm::setThrottle(2000);
  delay(1000);  // Wait for 1 second

  // Back to neutral
  pwm::setThrottle(1500);
  delay(3000);  // Wait for 1 second

  //set it to 1700 for 3 seconds
  pwm::setThrottle(1600);
  delay(2000);
  pwm::setThrottle(1700);
  delay(2000); 
  pwm::setThrottle(1800);
  delay(2000); 
  pwm::setThrottle(1900);
  delay(2000); 
  pwm::setThrottle(2000);
  delay(5000);

  // Back to neutral
  pwm::setThrottle(1500);
*/
  
  
  

}  




void loop() {




  startTime = micros();

  //currently blocking!!
  rcon::RadioLoopState state = rcon::radio_loop(radio);

  endTime = micros();
  elapsedTime = endTime - startTime;
  
  //Serial.println("RF24 radio_loop function: " + String(elapsedTime) + " microseconds");


  if (state != rcon::RadioLoopState::CONNECTED_IDLE) {
    //rcon::print_radio_loop_state(state);

  }




  rcon::HandleCommand(rf_incoming_buffer);
  pwm::update_smooth_pwms();




  if (disp::should_update_display()) {
    disp::update_stats_V1(radioConnected, millis() - last_packet_timestamp_millis, millis(), rf_incoming_buffer);
  }
 


} 
