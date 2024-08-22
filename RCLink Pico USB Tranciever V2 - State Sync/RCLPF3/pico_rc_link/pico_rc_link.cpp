#define TUD_OPT_HIGH_SPEED 1


#include "pico/stdlib.h"  // printf(), sleep_ms(), getchar_timeout_us(), to_us_since_boot(), get_absolute_time()
#include <tusb.h>         // tud_cdc_connected()
#include <RF24.h>         // RF24 radio object
#include <stdio.h>
#include "pico/time.h"
#include "pico/multicore.h"
#include "pico/mutex.h" //mutexes and thread launching

#define SERIAL_RECV_TIMEOUT_MS 100
#define RF_RECV_TIMEOUT_MS 25


#define SERIAL_LOOP_DELAY_MS 3 // 500packet/sec max speed
#define RF_LOOP_DELAY_MS 3


#define MAX_STRING_LENGTH 32


//macro for printf_safe which calls printf only if the serial port is connected
#define printf_safe(...) if (tud_cdc_connected()) {printf(__VA_ARGS__);}


mutex telemetry_mutex;
mutex controller_mutex;


//synced by rf24 and serial threads respectively
char telemetryState[32];
char controllerState[32];


#define DEBUG_LED_PIN 25




/* Variables for core 0 only */
SPI spi;
RF24 radio(6, 5);
bool connected = false;



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

    gpio_put(DEBUG_LED_PIN, 1);
    sleep_us(150);
    gpio_put(DEBUG_LED_PIN, 0);
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
            mutex_enter_blocking(&telemetry_mutex);
            strcpy(telemetryState, "ERROR: Initiate Con. Failed");
            mutex_exit(&telemetry_mutex);
        }
        sleep_ms(100);
        return;

    }  



    mutex_enter_blocking(&controller_mutex);
    strcpy(rf_outgoing_buffer, controllerState);
    mutex_exit(&controller_mutex);




    //send the data
    bool report = radio.write(rf_outgoing_buffer, sizeof(controllerState));


    

    if (report) {
        //printf_safe("Successfully sent data to remote tranciever\n");
    }
    else {
        //printf_safe("ERROR: Transmission Failed\n");
        
        mutex_enter_blocking(&telemetry_mutex);
        strcpy(telemetryState, "ERROR: Transmission Failed");
        mutex_exit(&telemetry_mutex);
        
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
            mutex_enter_blocking(&telemetry_mutex);
            strcpy(telemetryState, rf_incoming_buffer);
            mutex_exit(&telemetry_mutex);


            break;
        }
    }

    //set debug pin to high after receiving
    gpio_put(DEBUG_LED_PIN, 1);

    if (to_ms_since_boot(get_absolute_time()) - timeout_start >= RF_RECV_TIMEOUT_MS){
        //printf_safe("ERROR: RF Data Timeout\n");

        mutex_enter_blocking(&telemetry_mutex);
        strcpy(telemetryState, "ERROR: RF Data Timeout");
        mutex_exit(&telemetry_mutex);

        connected = false;
        return;


    }

    gpio_put(DEBUG_LED_PIN, 0);



    

    sleep_ms(RF_LOOP_DELAY_MS);


}

    



/* RF24 Radio Communication Thread */
void core0_entry()
{
    spi.begin(spi0, 2, 3, 4); // spi0 or spi1 bus, SCK, TX, RX
    uint8_t address[][6] = {"1Tnsy", "2Pico"};
    bool radioNumber = 0;


    strcpy(controllerState, "NO_LINK");

    // initialize the transceiver on the SPI bus
    while (!radio.begin()) {
        //printf_safe("ERROR: Radio Hardware Failure\n");
        
        mutex_enter_blocking(&telemetry_mutex);
        strcpy(telemetryState, "ERROR: Radio Hardware Failure");
        mutex_exit(&telemetry_mutex);

        sleep_ms(200);

    }

    radio.setChannel(118); // 2.518 GHz




    
    //radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.
    radio.setPALevel(RF24_PA_HIGH, 0);//set to max power and enable LNA

    radio.setPayloadSize(sizeof(rf_outgoing_buffer)); // float datatype occupies 4 bytes
    radio.openWritingPipe(address[radioNumber]); // always uses pipe 0

    radio.openReadingPipe(1, address[!radioNumber]); // using pipe 1
    radio.stopListening(); // put radio in TX mode

    //radio.printPrettyDetails();

    //set the debug pin
    gpio_init(DEBUG_LED_PIN);
    gpio_set_dir(DEBUG_LED_PIN, GPIO_OUT);


    while (true)
    {
        core0_rf_loop();
    }


}


void core1_serial_loop()
{


    //clear serial bnuffer
    for (int i = 0; i < MAX_STRING_LENGTH; i++){
        serial_tmp_buffer[i] = '\0';
    }

    //send one item from the queue, lock
    mutex_enter_blocking(&telemetry_mutex);
    strcpy(serial_tmp_buffer, telemetryState);
    mutex_exit(&telemetry_mutex);

    

    //send the data
    printf_safe("%s\n", serial_tmp_buffer);

    

    long int timeout_start = to_ms_since_boot(get_absolute_time());
    long int serial_recv_pos = 0;


    //clear serial bnuffer
    for (int i = 0; i < MAX_STRING_LENGTH; i++){
        serial_tmp_buffer[i] = '\0';
    }

    while (to_ms_since_boot(get_absolute_time()) - timeout_start < SERIAL_RECV_TIMEOUT_MS)
    {
        if (tud_cdc_available()){
            char inByte = getchar();
            serial_tmp_buffer[serial_recv_pos] = inByte;

            serial_recv_pos++;
            if (inByte == '\n' || inByte == '\r'){
                serial_tmp_buffer[serial_recv_pos] = '\0';
                break;
            }
            if (serial_recv_pos >= 32){
                break;
                //printf_safe("ERROR: Serial overflow\n");
            }
        }
    }

    //echo back the data
    //printf_safe("ECHO:\n", serial_tmp_buffer);



    if (serial_recv_pos == 0) { // If no data has been read

        

        if (!tud_cdc_available())
        {
            mutex_enter_blocking(&controller_mutex);
      
            strcpy(controllerState, "PICO_NO_SERIAL");

            mutex_exit(&controller_mutex);

        }

        else
        {
            printf_safe("ERROR: Serial Timeout\n");
            //strcpy(rf_outgoing_buffer, "NO_DATA");

            mutex_enter_blocking(&controller_mutex);
  
            strcpy(controllerState, "PICO_SERIAL_ERROR");

            mutex_exit(&controller_mutex);
        }
    }
    
    else
    {
        //rf_outgoing_buffer[serial_recv_pos] = '\0'; // Ensure null termination

        mutex_enter_blocking(&controller_mutex);
        //push_circular(&serial_outgoing_buffer, serial_tmp_buffer);
        strcpy(controllerState, serial_tmp_buffer);
        mutex_exit(&controller_mutex);

    }

    sleep_ms(SERIAL_LOOP_DELAY_MS);

}


/* Serial Communication Thread */
void core1_entry()
{


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
    mutex_init(&controller_mutex);
    mutex_init(&telemetry_mutex);

    multicore_launch_core1(core1_entry);

    core0_entry();

    return 0; // we will never reach this
}
