#include <Arduino.h>
#include "RF24.h"
#include "RadioController.hpp"

#define LED_PIN 13


RF24 radio(CE_PIN, CSN_PIN);





void setup() {
  rcon::radio_setup(radio);



}  




void loop() {

  rcon::RadioLoopState state = rcon::radio_loop(radio);




} 
