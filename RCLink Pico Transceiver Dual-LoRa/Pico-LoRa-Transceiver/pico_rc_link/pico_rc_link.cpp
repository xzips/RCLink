#include <string.h>
#include <pico/stdlib.h>
#include "LoRa-RP2040.h"





int main() {

	stdio_init_all();




	sleep_ms(8000);


    LoRa1.setPins(8, 9, 7, 8);
    LoRa2.setPins(3, 4, 2, 3);


	printf("Starting LoRa\n");

	if (!LoRa1.begin(446E6)) {
		printf("Starting LoRa1 failed!\n");
		while (1);
	}

    if (!LoRa2.begin(446E6)) {
        printf("Starting LoRa2 failed!\n");
        while (1);
    }


   // LoRa1.dumpRegisters();
    //LoRa2.dumpRegisters();


	uint8_t counter = 0;

	while (1) {

		printf("Sending packet: ");
		printf("%d \n",counter);
		// send packet
		LoRa1.beginPacket();
		LoRa1.print("Test #");
		LoRa1.print(counter);
		LoRa1.endPacket();


        sleep_ms(20);

       

        int packetSize = LoRa2.parsePacket();
        if (packetSize) {
            // received a packet
            printf("Received packet: '");

            // read packet
            while (LoRa2.available()) {
                unsigned char byte = LoRa2.read();

                 printf("%c", byte);
            }

            printf("\n");

            // print RSSI of packet
            //printf("' with RSSI \n");
            //printf((char*)LoRa2.packetRssi(),"\n");
        }


		counter++;
		sleep_ms(10);

        
        //printf("Loop done, starting again\n");
		
  }
  return 0;
}