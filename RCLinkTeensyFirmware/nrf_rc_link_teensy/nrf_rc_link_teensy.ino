
#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include <Arduino.h>

#define CE_PIN 0
#define CSN_PIN 1


#define LED_PIN 13

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

  pinMode(LED_PIN, OUTPUT);


  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }
  digitalWrite(LED_PIN, HIGH);   // set the LED on

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));

    pinMode(LED_PIN, OUTPUT);
    while (true) {
      digitalWrite(LED_PIN, HIGH); 
      delay(500);           
      digitalWrite(LED_PIN, LOW); 
      delay(500);
      Serial.println(F("radio hardware is not responding, please reboot"));            
    } 

  }

  Serial.println(F("Radio begun"));    

  //pinMode(LED_PIN, OUTPUT);
  //digitalWrite(LED_PIN, HIGH); 




  radio.setPALevel(RF24_PA_LOW);


  radio.setPayloadSize(32);

  radio.openWritingPipe(address[radioNumber]);


  radio.openReadingPipe(1, address[!radioNumber]);


  Serial.println(F("Beginning listening"));    
  radio.startListening();


  printf_begin();
  radio.printPrettyDetails();

  Serial.println(F("Setting LED pin mode"));    
  pinMode(LED_PIN, OUTPUT);

  //flash 

  for (int i = 0; i < 6; i++)
  {
      digitalWrite(LED_PIN, HIGH); 
      delay(50);           
      digitalWrite(LED_PIN, LOW); 
      delay(50);
  }

  Serial.println(F("Radio hardware fully initialized, proceeding..."));    

}  

bool connected = false;


void loop() {

  
  digitalWrite(LED_PIN, HIGH); 
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
  sprintf(rf_outgoing_buffer, "%ld", millis());

  //strcpy(rf_outgoing_buffer, "Hello from Teensy");

  bool report = radio.write(&rf_outgoing_buffer, 32);

  if (report) {
    Serial.println("Successfully sent data to remote tranciever");
  }
  else {
    Serial.println("Data transmission failed or timed out");
    connected = false;
    return;
  }





} 
