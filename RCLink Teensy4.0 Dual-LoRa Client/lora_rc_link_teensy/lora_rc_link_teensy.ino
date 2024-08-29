#include <SPI.h>
#include "LoRa.h"

void setup() {
  Serial.begin(9600);
  while (!Serial);

  //Serial.println("LoRa Receiver");

  SPI.begin();


  //test spi pins

  

  //ss, reset, dio
  LoRa1.setPins(9, 7, 5);

  LoRa2.setPins(8, 6, 4);




  if (!LoRa1.begin(446E6)) {
    Serial.println("Starting LoRa 1 failed!");
    while (1);
  }

  if (!LoRa2.begin(446E6)) {
    Serial.println("Starting LoRa 2 failed!");
    while (1);
  }



}

void loop() {
  // try to parse packet
  int packetSize = LoRa1.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa1.available()) {
      Serial.print((char)LoRa1.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa1.packetRssi());
  }
}