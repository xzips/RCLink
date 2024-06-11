#include "pico/stdlib.h"  // printf(), sleep_ms(), getchar_timeout_us(), to_us_since_boot(), get_absolute_time()
#include <tusb.h>         // tud_cdc_connected()
#include <RF24.h>         // RF24 radio object
#include <stdio.h>
#include "pico/time.h"
#include "pico/multicore.h"
#include "pico/mutex.h" //mutexes and thread launching

#define SERIAL_RECV_TIMEOUT_MS 100
#define RF_RECV_TIMEOUT_MS 100


#define SERIAL_LOOP_DELAY_MS 0 // 500packet/sec max speed
#define RF_LOOP_DELAY_MS 16

#define SIZE_OF_BUFFER 2
#define MAX_STRING_LENGTH 32

#define CORE0_DEBUG_PIN 0
#define CORE1_DEBUG_PIN 1

//macro for printf_safe which calls printf only if the serial port is connected
#define printf_safe(...) if (tud_cdc_connected()) {printf(__VA_ARGS__);}

/*Mutexes*/
//auto_init_mutex(mutex_serial_outgoing_buffer)
//auto_init_mutex(mutex_serial_incoming_buffer)


mutex mutex_serial_outgoing_buffer;
mutex mutex_serial_incoming_buffer;

//PLAN for new code: rf24 and all radio goes in the main/normal/base thread, and serial goes in second thread



/*Circular Buffer Code*/
typedef struct  {
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

CircularBuffer serial_outgoing_buffer;
CircularBuffer serial_incoming_buffer;

char rf_outgoing_buffer[MAX_STRING_LENGTH];
char rf_incoming_buffer[MAX_STRING_LENGTH];

char serial_tmp_buffer[MAX_STRING_LENGTH];


/*Variables for core 1 only*/

char serial_incoming_tmp[MAX_STRING_LENGTH];





void core0_rf_loop()
{
    //do a pattern of setting gpiuo pin to high then low for exactly 150us

    radio.stopListening();
    //sleep_us(130);

    gpio_put(CORE0_DEBUG_PIN, 1);
    sleep_us(150);
    gpio_put(CORE0_DEBUG_PIN, 0);
    sleep_us(10);


    //pico always starts transmission, and the other one probably isn't ready yet, so we need to wait for a response
    if (!connected){

        strcpy(rf_outgoing_buffer, "REQUEST_CONNECTION");

        bool report = radio.write(rf_outgoing_buffer, sizeof(rf_outgoing_buffer));

        
        if (report) {
            connected = true;
        }
        else {
            //printf_safe("ERROR: Initiate Connection Failed\n");
            mutex_enter_blocking(&mutex_serial_incoming_buffer);
            push_circular(&serial_incoming_buffer, "ERROR: Initiate Connection Failed");
            mutex_exit(&mutex_serial_incoming_buffer);
        }

    
        sleep_ms(100);
        return;

    }  


    //lock mutex and check if there is any data to pop
    mutex_enter_blocking(&mutex_serial_outgoing_buffer);
    char serial_outgoing_tmp[MAX_STRING_LENGTH];

    if (pop_circular(&serial_outgoing_buffer, serial_outgoing_tmp) == 0){
        strcpy(rf_outgoing_buffer, serial_outgoing_tmp);
    }
    else{
        strcpy(rf_outgoing_buffer, "NO_DATA");
    }

    mutex_exit(&mutex_serial_outgoing_buffer);

    //set the core 0 debug pin to high when transmitting
    gpio_put(CORE0_DEBUG_PIN, 1);

    //send the data
    bool report = radio.write(rf_outgoing_buffer, sizeof(rf_outgoing_buffer));


    //set debug pin to low after transmission
    gpio_put(CORE0_DEBUG_PIN, 0);
    

    if (report) {
        //printf_safe("Successfully sent data to remote tranciever\n");
    }
    else {
        //printf_safe("ERROR: Transmission Failed\n");
        
        mutex_enter_blocking(&mutex_serial_incoming_buffer);
        push_circular(&serial_incoming_buffer, "ERROR: Transmission Failed");
        mutex_exit(&mutex_serial_incoming_buffer);
        
        connected = false;
        return;
    }



    //auto ack already done, now we wait for a payload to come in, so switch to RX mode and wait 130us so the other side has a chance to send their data
    radio.startListening();
    sleep_us(130);


    uint8_t pipe;

    long int timeout_start = to_ms_since_boot(get_absolute_time());


    

    while (to_ms_since_boot(get_absolute_time()) - timeout_start < RF_RECV_TIMEOUT_MS){
        if (radio.available(&pipe)) {               // is there a payload? get the pipe number that recieved it
            uint8_t bytes = radio.getPayloadSize(); // get the size of the payload
            radio.read(rf_incoming_buffer, bytes);            // fetch payload from FIFO


            //printf_safe("%s\n", rf_incoming_buffer);

            //lock mutex and push the data to the incoming buffer
            mutex_enter_blocking(&mutex_serial_incoming_buffer);
            push_circular(&serial_incoming_buffer, rf_incoming_buffer);
            mutex_exit(&mutex_serial_incoming_buffer);


            break;
        }
    }

    //set debug pin to high after receiving
    gpio_put(CORE0_DEBUG_PIN, 1);

    if (to_ms_since_boot(get_absolute_time()) - timeout_start >= RF_RECV_TIMEOUT_MS){
        //printf_safe("ERROR: RF Data Timeout\n");

        mutex_enter_blocking(&mutex_serial_incoming_buffer);
        push_circular(&serial_incoming_buffer, "ERROR: RF Data Timeout");
        mutex_exit(&mutex_serial_incoming_buffer);

        connected = false;
        return;


    }

    
    //set debug pin to low after receiving
    gpio_put(CORE0_DEBUG_PIN, 0);

    sleep_ms(RF_LOOP_DELAY_MS);


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
        
        mutex_enter_blocking(&mutex_serial_incoming_buffer);
        push_circular(&serial_incoming_buffer, "ERROR: Radio Hardware Failure");
        mutex_exit(&mutex_serial_incoming_buffer);

        sleep_ms(200);

    }

    //if initialized, lock queue and pop all items to clear it
    mutex_enter_blocking(&mutex_serial_outgoing_buffer);
    while (pop_circular(&serial_outgoing_buffer, rf_outgoing_buffer) == 0);
    mutex_exit(&mutex_serial_outgoing_buffer);


    
    //radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.
    radio.setPALevel(RF24_PA_HIGH, 0);//set to max power and enable LNA

    radio.setPayloadSize(sizeof(rf_outgoing_buffer)); // float datatype occupies 4 bytes
    radio.openWritingPipe(address[radioNumber]); // always uses pipe 0

    radio.openReadingPipe(1, address[!radioNumber]); // using pipe 1
    radio.stopListening(); // put radio in TX mode

    //radio.printPrettyDetails();

    //set the debug pin
    gpio_init(CORE0_DEBUG_PIN);
    gpio_set_dir(CORE0_DEBUG_PIN, GPIO_OUT);


    while (true)
    {
        core0_rf_loop();
    }


}


void core1_serial_loop()
{



    //send one item from the queue, lock
    mutex_enter_blocking(&mutex_serial_incoming_buffer);
    //pop to serial_tmp_buffer
    if (pop_circular(&serial_incoming_buffer, serial_incoming_tmp) == 0){
        strcpy(serial_tmp_buffer, serial_incoming_tmp);
    }
    else{
        strcpy(serial_tmp_buffer, "RECV_BUF_EMPTY");
    }

    mutex_exit(&mutex_serial_incoming_buffer);

    //send the data
    printf_safe("%s\n", serial_tmp_buffer);

    

    long int timeout_start = to_ms_since_boot(get_absolute_time());
    long int serial_recv_pos = 0;

    while (to_ms_since_boot(get_absolute_time()) - timeout_start < SERIAL_RECV_TIMEOUT_MS)
    {
        if (tud_cdc_available()){
            char inByte = getchar();
            serial_tmp_buffer[serial_recv_pos] = inByte;

            serial_recv_pos++;
            if (inByte == '\n'){
                serial_tmp_buffer[serial_recv_pos] = '\0';
                break;
            }
            if (serial_recv_pos >= 32){
                break;
                //printf_safe("ERROR: Serial overflow\n");
            }
        }
    }



    if (serial_recv_pos == 0) { // If no data has been read

        

        if (!tud_cdc_available())
        {
            //printf_safe("ERROR: No Serial, sending PICO_NO_SERIAL\n");
            //strcpy(rf_outgoing_buffer, "PICO_NO_SERIAL");

            mutex_enter_blocking(&mutex_serial_outgoing_buffer);
            push_circular(&serial_outgoing_buffer, "PICO_NO_SERIAL");
            mutex_exit(&mutex_serial_outgoing_buffer);

        }

        else
        {
            printf_safe("ERROR: Serial Timeout\n");
            //strcpy(rf_outgoing_buffer, "NO_DATA");

            mutex_enter_blocking(&mutex_serial_outgoing_buffer);
            push_circular(&serial_outgoing_buffer, "NO_DATA");
            mutex_exit(&mutex_serial_outgoing_buffer);
        }
    }
    
    else
    {
        //rf_outgoing_buffer[serial_recv_pos] = '\0'; // Ensure null termination

        mutex_enter_blocking(&mutex_serial_outgoing_buffer);
        push_circular(&serial_outgoing_buffer, serial_tmp_buffer);
        mutex_exit(&mutex_serial_outgoing_buffer);

    }

    sleep_ms(SERIAL_LOOP_DELAY_MS);

}


/* Serial Communication Thread */
void core1_entry()
{
    //set the debug pin
    gpio_init(CORE1_DEBUG_PIN);


    // wait here until the CDC ACM (serial port emulation) is connected
    while (!tud_cdc_connected()) {
        sleep_ms(10);
    }



    while (true)
    {
        core1_serial_loop();
    }
    

}









int main()
{
    //set_sys_clock_khz(250000, true);

    stdio_init_all();

    //initialize mutexes
    mutex_init(&mutex_serial_outgoing_buffer);
    mutex_init(&mutex_serial_incoming_buffer);

    multicore_launch_core1(core1_entry);

    core0_entry();

    return 0; // we will never reach this
}
