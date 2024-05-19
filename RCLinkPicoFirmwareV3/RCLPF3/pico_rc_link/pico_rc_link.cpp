#include "pico/stdlib.h"  // printf(), sleep_ms(), getchar_timeout_us(), to_us_since_boot(), get_absolute_time()
#include <tusb.h>         // tud_cdc_connected()
#include <RF24.h>         // RF24 radio object
#include <stdio.h>
#include "pico/time.h"
#include "pico/multicore.h" //mutexes and thread launching

#define SERIAL_RECV_TIMEOUT_MS 1000
#define RF_RECV_TIMEOUT_MS 1000
#define DATA_LOOP_DELAY_MS 50
#define SIZE_OF_BUFFER 4
#define MAX_STRING_LENGTH 32

//macro for printf_safe which calls printf only if the serial port is connected
#define printf_safe(...) if (tud_cdc_connected()) {printf(__VA_ARGS__);}


//PLAN for new code: rf24 and all radio goes in the main/normal/base thread, and serial goes in second thread



/*Circular Buffer Code*/
typedef struct {
    char buffer[SIZE_OF_BUFFER][MAX_STRING_LENGTH];
    int writeIndex;
    int readIndex;
    int bufferLength;
} CircularBuffer;


int push_circular(CircularBuffer *cb, const char *value) {
    if (cb->bufferLength == SIZE_OF_BUFFER) {
        return -1; // Buffer is full
    }

    strncpy(cb->buffer[cb->writeIndex], value, MAX_STRING_LENGTH - 1);
    cb->buffer[cb->writeIndex][MAX_STRING_LENGTH - 1] = '\0'; // Ensure null-termination
    cb->writeIndex = (cb->writeIndex + 1) % SIZE_OF_BUFFER;
    cb->bufferLength++;

    return 0;
}

int pop_circular(CircularBuffer *cb, char *output) {
    if (cb->bufferLength == 0) {
        return -1; // Buffer is empty
    }

    strncpy(output, cb->buffer[cb->readIndex], MAX_STRING_LENGTH - 1);
    output[MAX_STRING_LENGTH - 1] = '\0'; // Ensure null-termination
    cb->readIndex = (cb->readIndex + 1) % SIZE_OF_BUFFER;
    cb->bufferLength--;

    return 0;
}



/* Variables for core 0 only */
SPI spi;
RF24 radio(6, 5);
bool connected = false;

CircularBuffer outgoing_buffer;
CircularBuffer incoming_buffer;


/*Mutexes*/
auto_init_mutex(mutex_outgoing_buffer)
auto_init_mutex(mutex_incoming_buffer)





//returns true if success, false if buffer is full
bool atomic_buffer_push(char **buffer, int *first, int *last, char *data)
{
    int next = (*last + 1) % PACKET_BUFFER_COUNT;
    if (next == *first)
    {
        return false;
    }

    strcpy(buffer[*last], data);
    *last = next;
    return true;
}


void clear_all_buffers()
{
    for (int i = 0; i < PACKET_BUFFER_COUNT; i++)
    {
        for (int j = 0; j < PACKET_BUFFER_SIZES; j++)
        {
            rf_outgoing_buffer[i][j] = '\0';
            rf_incoming_buffer[i][j] = '\0';
        }
    }
}


/* RF24 Radio Communication Thread */
void core0_entry()
{
    spi.begin(spi0, 2, 3, 4); // spi0 or spi1 bus, SCK, TX, RX
    uint8_t address[][6] = {"1Tnsy", "2Pico"};
    bool radioNumber = 0;

    // initialize the transceiver on the SPI bus
    while (!radio.begin()) {
        //printf_safe("ERROR: Radio Hardware Failure\n");
        TODO

    }

    
    //radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.
    radio.setPALevel(RF24_PA_HIGH, 0);//set to max power and enable LNA

    radio.setPayloadSize(sizeof(rf_outgoing_buffer)); // float datatype occupies 4 bytes
    radio.openWritingPipe(address[radioNumber]); // always uses pipe 0

    radio.openReadingPipe(1, address[!radioNumber]); // using pipe 1
    radio.stopListening(); // put radio in TX mode

    //radio.printPrettyDetails();

    while (true)
    {
        ...
    }


}


/* Serial Communication Thread */
void core1_entry()
{
    // wait here until the CDC ACM (serial port emulation) is connected
    while (!tud_cdc_connected()) {
        sleep_ms(10);
    }

}





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
            printf_safe("ERROR: Initiate Connection Failed\n");


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
                printf_safe("ERROR: Serial buffer overflow\n");
            }

        }
    }



    if (serial_recv_pos == 0) { // If no data has been read
        if (!tud_cdc_available())
        {
            printf_safe("ERROR: No Serial, sending PICO_NO_SERIAL\n");
            strcpy(rf_outgoing_buffer, "PICO_NO_SERIAL");
        }

        else
        {
            printf_safe("ERROR: Serial Timeout, sending NO_DATA\n");
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
        printf_safe("ERROR: Transmission Failed\n");
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
        printf_safe("ERROR: RF Data Timeout\n");
        connected = false;
        return;
    }

    //now we return to the main loop and start the process over again after waiting for 500 ms
    sleep_ms(DATA_LOOP_DELAY_MS);
}

    




int main()
{
    stdio_init_all();
    clear_all_buffers();

    while (!setup()) { // if radio.begin() failed
        // hold program in infinite attempts to initialize radio
    }
    while (true) {
        loop();
    }

    multicore_launch_core1(core1_entry);

    core0_entry();

    return 0; // we will never reach this
}
