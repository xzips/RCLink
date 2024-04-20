/*
Aspen Erlandsson Brisebois, 2024, theaspen.ca
 */



/*
General paradigm approach:
- build a robust two way serial comm link between main desktop app and pico firmware
- add the nrf module to the pico and run it on top of the serial comm link afterwords

- all commands sent either way are as strings, and ACK must be sent back to confirm receipt of data or command

- we can either get ACK-RX which mean the device is happy to continue recieving one way, or ACK-TX which means the device wants to also send something, and the other side should be ready to recieve
- functionally they take turns sending packets if data is going both ways, and if one side is done sending data, it sends an ACK-RX to let the other side know it can send data
- for example if both sides are transmitting, they will both be sending ACK-TX back and forth to each other
- if there is a transmission error on either device, both devices should requeue the packet, but the PC device should send an ACK-RX to let the pico know it can send again, i.e. being on the defensive, yielding side of the transmission
- pico should try to resend packet if failed

- strict timeout of 5ms for ACK, as well as waiting if we have stuff to send and other side sent an ACK-TX

- later when transferring data to and from nrf, we should do any SPI operations one at a time and check for transmissions from pc in between, so we dont exceed the timeout if a list of nrf operations takes a while together

*/



/*

- new paradigm: legit just send messages back and forth, typically we get ACK-STB for standby, ACK-RX for we recieved a message, but we allow longer messages

*/


#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include <tusb.h>
#include <stdlib.h>
#include <string.h>
#include "lib/nrf24l01/nrf24_driver.h"


#define MAX_MESSAGE_LENGTH 1024
#define ACK_TIMEOUT_MS 5000


//1 ack-tx, 0 ack-rx, -1 message not terminated with newline, -2 ack message too long, -3 unknown ack reply received, -4 timeout waiting for ack
int send_message(char* message, char* rx_buffer){
    //verify message contains correct end character (newline)
    if (message[strlen(message)-1] != '\n'){
        return -1;
    }

    //send message
    printf("%s",message);

    long int start = to_ms_since_boot(get_absolute_time());

    //wait for ack
    unsigned int ack_pos = 0;
    while (to_ms_since_boot(get_absolute_time()) - start < ACK_TIMEOUT_MS){
        if (tud_cdc_available()){
            char inByte = getchar();
            rx_buffer[ack_pos] = inByte;
            ack_pos++;
            if (inByte == '\n'){
                rx_buffer[ack_pos] = '\0';
                break;
            }

            if (ack_pos >= MAX_MESSAGE_LENGTH){
                return -2;
            }

        }
    }

    //if time exceeded, return -4 and for now print timeout
    if (to_ms_since_boot(get_absolute_time()) - start < ACK_TIMEOUT_MS){
        //printf("Timeout waiting for ACK\n");
        return -4;
    }

    //if ACK-TX, return 1
    if (strcmp(rx_buffer,"ACK-TX\n") == 0){
        return 1;
    }

    //if ACK-RX, return 0
    else if (strcmp(rx_buffer,"ACK-RX\n") == 0){
        return 0;
    }


    else {
        return -3;
    }

}


int recieve_loop(char* tx_buffer, char* rx_buffer){
    //this should basically just wait in a loop for a message to come in, and simultanoussly check if we have a message to send



}



int main()
{
    stdio_init_all();

    char* rx_buffer = (char*)malloc(MAX_MESSAGE_LENGTH);
    char* tx_buffer = (char*)malloc(MAX_MESSAGE_LENGTH);

    char base_str[] = "Hello, World! - Pico";
    int cur_num = 0;


    while (!tud_cdc_connected())
    {
           sleep_ms(100); 
    }


    printf("tud_cdc_connected()\n");
    while (true)
    {
        sprintf(tx_buffer, "%s %d\n", base_str, cur_num);



        int msg_status = send_message(tx_buffer, rx_buffer);
        sleep_ms(1000); 
        

        cur_num++;

    }

    return 0;
} 