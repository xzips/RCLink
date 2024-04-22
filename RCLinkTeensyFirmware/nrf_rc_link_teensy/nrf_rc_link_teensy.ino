/*
 * See documentation at https://nRF24.github.io/RF24
 * See License information at root directory of this library
 * Author: Brendan Doherty (2bndy5)
 */

/**
 * A simple example of sending data from 1 nRF24L01 transceiver to another.
 *
 * This example was written to be used on 2 devices acting as "nodes".
 * Use the Serial Monitor to change each node's behavior.
 */
#include <SPI.h>
#include "printf.h"
#include "RF24.h"

#define CE_PIN 0
#define CSN_PIN 1
// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);

// Let these addresses be used for the pair
uint8_t address[][6] = { "1Tnsy", "2Pico" };
bool radioNumber = 1;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit

// Used to control whether this node is sending or receiving
bool role = false;  // true = TX role, false = RX role


char rf_outgoing_buffer[32];
char rf_incoming_buffer[32];

void clear_outgoing_buffer()
{
    for (int i = 0; i < 32; i++) {
        rf_outgoing_buffer[i] = 0;
    }

}

void clear_incoming_buffer()
{
    for (int i = 0; i < 32; i++) {
        rf_incoming_buffer[i] = 0;
    }

}

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }


  // Set the PA Level low to try preventing power supply related problems
  // because these examples are likely run with nodes in close proximity to
  // each other.
  radio.setPALevel(RF24_PA_LOW);  // RF24_PA_MAX is default.

  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(32);  // float datatype occupies 4 bytes

  // set the TX address of the RX node into the TX pipe
  radio.openWritingPipe(address[radioNumber]);  // always uses pipe 0

  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(1, address[!radioNumber]);  // using pipe 1

  // additional setup specific to the node's role

  radio.startListening();  // put radio in RX mode


  //printf_begin();             // needed only once for printing details
  radio.printPrettyDetails(); // (larger) function that prints human readable data

}  

bool connected = false;


void loop() {
  radio.startListening();
  //sleep 130us
  delayMicroseconds(130);


  
  uint8_t pipe;
  
  if (!connected){
    if (radio.available(&pipe)) {              // is there a payload? get the pipe number that recieved it
      uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
      radio.read(&rf_incoming_buffer, bytes);             // fetch payload from FIFO

      
      connected = true;
      Serial.println("Connected to remote tranciever");
   


    }
  }

  if (!connected) return;


  //assuming connected

  //listen for the first incoming message
  if (radio.available(&pipe)) {              // is there a payload? get the pipe number that recieved it
    uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
    radio.read(&rf_incoming_buffer, bytes);             // fetch payload from FIFO

    //print the incoming message
    Serial.print("Received: ");
    Serial.println(rf_incoming_buffer);

    //clear the incoming buffer
    clear_incoming_buffer();

  }

  else{
    return;
  }

  //wait 500ms before sending a return message
  delay(500);

  radio.stopListening();

  clear_outgoing_buffer();
  //send a return message, this would in theory be telemtry and other information data, for now print the internal clock
  //sprintf(rf_outgoing_buffer, "%ld", millis());

  strcpy(rf_outgoing_buffer, "Hello from Teensy");

  bool report = radio.write(&rf_outgoing_buffer, 32);

  if (report) {
    Serial.println("Successfully sent data to remote tranciever");
  }
  else {
    Serial.println("Data transmission failed or timed out");
    connected = false;
    return;
  }





}  // loop
