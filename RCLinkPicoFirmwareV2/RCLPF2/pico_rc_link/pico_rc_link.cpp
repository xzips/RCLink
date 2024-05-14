#include "pico/stdlib.h"  // printf(), sleep_ms(), getchar_timeout_us(), to_us_since_boot(), get_absolute_time()
#include "pico/bootrom.h" // reset_usb_boot()
#include <tusb.h>         // tud_cdc_connected()
#include <RF24.h>         // RF24 radio object
SPI spi;

#include <stdio.h>
#include "pico/time.h"


#define SERIAL_RECV_TIMEOUT_MS 300
#define RF_RECV_TIMEOUT_MS 5000
#define DATA_LOOP_DELAY_MS 0


//macro for printf_safe which calls printf only if the serial port is connected
#define printf_safe(...) if (tud_cdc_connected()) {printf(__VA_ARGS__);}




RF24 radio(6, 5);



float payload = 0.0;

//max dyn payload size is 32 bytes
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


bool setup()
{


    //initialize the buffer and serial 
    clear_outgoing_buffer();
    clear_incoming_buffer();
   


    //initialize and setup the nrf24l01 radio
    spi.begin(spi0, 2, 3, 4); // spi0 or spi1 bus, SCK, TX, RX


    uint8_t address[][6] = {"1Tnsy", "2Pico"};
    bool radioNumber = 0;

    // wait here until the CDC ACM (serial port emulation) is connected
    while (!tud_cdc_connected()) {
        sleep_ms(10);
    }

    // initialize the transceiver on the SPI bus
    if (!radio.begin()) {
        printf_safe("radio hardware is not responding!!\n");
        return false;
    }



    radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.

    radio.setPayloadSize(sizeof(rf_outgoing_buffer)); // float datatype occupies 4 bytes

    // set the TX address of the RX node into the TX pipe
    radio.openWritingPipe(address[radioNumber]); // always uses pipe 0

    // set the RX address of the TX node into a RX pipe
    radio.openReadingPipe(1, address[!radioNumber]); // using pipe 1

    radio.stopListening(); // put radio in TX mode

    //radio.printPrettyDetails();



    return true;

}

bool connected = false;





void loop()
{

    //put the radio in TX mode
    radio.stopListening();
    sleep_us(130);

    //pico always starts transmission, and the other one probably isn't ready yet, so we need to wait for a response

    if (!connected){

        strcpy(rf_outgoing_buffer, "REQUEST_CONNECTION");

        bool report = radio.write(rf_outgoing_buffer, sizeof(rf_outgoing_buffer));

        if (report) {
            //printf_safe("Successfully connected to remote tranciever\n");
            connected = true;
        }
        else {
            printf_safe("Connection request failed or timed out\n");
        }

        sleep_ms(100);

        return;

    }  

    //if we are connected, we can start sending data, check the serial buffer for data to send

    long int timeout_start = to_ms_since_boot(get_absolute_time());

    long int serial_recv_pos = 0;

    while (to_ms_since_boot(get_absolute_time()) - timeout_start < SERIAL_RECV_TIMEOUT_MS)
    {
        if (tud_cdc_available()){
            char inByte = getchar();
            rf_outgoing_buffer[serial_recv_pos] = inByte;
            serial_recv_pos++;
            if (inByte == '\n'){
                rf_outgoing_buffer[serial_recv_pos] = '\0';
                
                break;
            }

            if (serial_recv_pos >= 32){
                break;
                printf_safe("Serial buffer overflow\n");
            }

        }
    }

    /*
    if (to_ms_since_boot(get_absolute_time()) - timeout_start >= SERIAL_RECV_TIMEOUT_MS){
        printf_safe("Timeout waiting for serial data, sending NO_DATA\n");
        strcpy(rf_outgoing_buffer, "NO_DATA");
    }

    if (!tud_cdc_available())
    {
        //serial not available, send PICO_NO_SERIAL
        strcpy(rf_outgoing_buffer, "PICO_NO_SERIAL");
    }*/

    if (serial_recv_pos == 0) { // If no data has been read
        if (!tud_cdc_available())
        {
            printf_safe("Serial not available, sending PICO_NO_SERIAL\n");
            strcpy(rf_outgoing_buffer, "PICO_NO_SERIAL");
        }

        else
        {
            printf_safe("Timeout waiting for serial data, sending NO_DATA\n");
            strcpy(rf_outgoing_buffer, "NO_DATA");
        }
    }
    
    else
    {

        rf_outgoing_buffer[serial_recv_pos] = '\0'; // Ensure null termination
    }




    //send the data
    bool report = radio.write(rf_outgoing_buffer, sizeof(rf_outgoing_buffer));

    if (report) {
        //printf_safe("Successfully sent data to remote tranciever\n");
    }
    else {
        printf_safe("Data transmission failed or timed out\n");
        connected = false;
        return;
    }

    //auto ack already done, now we wait for a payload to come in, so switch to RX mode and wait 130us so the other side has a chance to send their data
    radio.startListening();
    sleep_us(130);


    uint8_t pipe;

    timeout_start = to_ms_since_boot(get_absolute_time());

    while (to_ms_since_boot(get_absolute_time()) - timeout_start < RF_RECV_TIMEOUT_MS){
        if (radio.available(&pipe)) {               // is there a payload? get the pipe number that recieved it
            uint8_t bytes = radio.getPayloadSize(); // get the size of the payload
            radio.read(rf_incoming_buffer, bytes);            // fetch payload from FIFO

    

            // print the size of the payload, the pipe number, payload's value
            //printf_safe("Received %d bytes on pipe %d: %s\n", bytes, pipe, rf_incoming_buffer);
            
            //print payload
            printf_safe("%s\n", rf_incoming_buffer);

            break;
        }
    }

    if (to_ms_since_boot(get_absolute_time()) - timeout_start >= RF_RECV_TIMEOUT_MS){
        printf_safe("Timeout waiting for RF data\n");
        connected = false;
        return;
    }

    //now we return to the main loop and start the process over again after waiting for 500 ms
    sleep_ms(DATA_LOOP_DELAY_MS);
}

    




int main()
{
    stdio_init_all(); // init necessary IO for the RP2040

    while (!setup()) { // if radio.begin() failed
        // hold program in infinite attempts to initialize radio
    }
    while (true) {
        loop();
    }
    return 0; // we will never reach this
}
