#include <Arduino.h>
#include "RF24.h"
#include "RadioController.hpp"
#include "OLEDController.hpp"



unsigned long startTime;
unsigned long endTime;
unsigned long elapsedTime;

RF24 radio(CE_PIN, CSN_PIN);



void setup() {
  rcon::radio_setup(radio);
  disp::setup_display();

  last_packet_timestamp_millis = millis();

}  




void loop() {

  
  rcon::RadioLoopState state = rcon::radio_loop(radio);
  

  

  if (state != rcon::RadioLoopState::CONNECTED_IDLE) {
    //rcon::print_radio_loop_state(state);
    

  }

  //startTime = micros();

  if (disp::should_update_display()) {
    disp::update_stats_V1(radioConnected, millis() - last_packet_timestamp_millis, millis(), rf_incoming_buffer);
  }
 
  
  //endTime = micros();

  //elapsedTime = endTime - startTime;
  //Serial.println("OLED Update Time: " + String(elapsedTime) + " microseconds");
  

  //delay(5);


} 
