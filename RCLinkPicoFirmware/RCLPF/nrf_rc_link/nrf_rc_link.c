/*
Aspen Erlandsson Brisebois, 2024, theaspen.ca
 */


//The gets(char* str) function reads a line from stdin and stores it into the string pointed to by str. It stops when either the newline character is read or when an EOF is reached, whichever comes
//The puts(char* str) function writes a string to stdout up to but not including the null character. A newline character is appended to the output string.

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


#include <stdio.h>
#include "pico/stdlib.h"
#include <tusb.h>
#define MAX_MESSAGE_LENGTH 12
int main()
{
    stdio_init_all();
    while (!tud_cdc_connected())
    {
        sleep_ms(100);
    }
    printf("tud_cdc_connected()\n");
    while (true)
    {
        //Check to see if anything is available in the serial receive buffer
        while (tud_cdc_available()){
            //Create a place to hold the incoming message
            static char message[MAX_MESSAGE_LENGTH];
            static unsigned int message_pos = 0;
            //Read the next available byte in the serial receive buffer
            char inByte = getchar();
            //Message coming in (check not terminating character) and guard for over message size
            if ( inByte != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1) ) {
                //Add the incoming byte to our message
                message[message_pos] = inByte;
                message_pos++;
            }
            //Full message received...
            else{
                //Add null character to string
                message[message_pos] = '\0';
                printf("%s\n",message);
                //Reset for the next message
                message_pos = 0;
            }
        }
    }

    return 0;
} 