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
  
  pwm::add_smooth_pwm(15, 0, MS24_SPEED_DEG_PER_SEC);

  /*
  delay(1000);  // Wait for 1 second

  setThrottle(1500);
  delay(1000);  // Wait for 1 second

  // Set full throttle
  setThrottle(2000);
  delay(1000);  // Wait for 1 second

  // Back to neutral
  setThrottle(1500);
  delay(3000);  // Wait for 1 second

  //set it to 1700 for 3 seconds
  setThrottle(1600);
  delay(2000);
  setThrottle(1700);
  delay(2000); 
  setThrottle(1800);
  delay(2000); 
  setThrottle(1900);
  delay(2000); 
  setThrottle(2000);
  delay(8000);

  // Back to neutral
  setThrottle(1500);

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
