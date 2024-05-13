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

  //startTime = micros();
  rcon::RadioLoopState state = rcon::radio_loop(radio);
  //endTime = micros();

  //elapsedTime = endTime - startTime;

  if (state != rcon::RadioLoopState::CONNECTED_IDLE) {
    rcon::print_radio_loop_state(state);
   // Serial.println("Radio Loop Time: " + String(elapsedTime) + " microseconds");

  }

  disp::update_stats_V1(radioConnected, millis() - last_packet_timestamp_millis, millis(), rf_incoming_buffer);



  

  delay(100);


} 
