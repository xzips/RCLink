#include <Arduino.h>
#include "RF24.h"
#include "RadioController.hpp"


unsigned long startTime;
unsigned long endTime;
unsigned long elapsedTime;

RF24 radio(CE_PIN, CSN_PIN);






void setup() {
  rcon::radio_setup(radio);

}  




void loop() {

  startTime = micros();
  rcon::RadioLoopState state = rcon::radio_loop(radio);
  endTime = micros();

  elapsedTime = endTime - startTime;

  if (state != rcon::RadioLoopState::CONNECTED_IDLE) {
    rcon::print_radio_loop_state(state);
    Serial.println("Radio Loop Time: " + String(elapsedTime) + " microseconds");

  }

  

  delay(100);


} 
