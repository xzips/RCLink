#include <string.h>
#include <pico/stdlib.h>
#include <tusb.h> 
#include "LoRa-RP2040.h"
#include "pico/multicore.h"
#include "pico/mutex.h"
#include <stdio.h>

#define SERIAL_RECV_TIMEOUT_MS 100
#define RF_RECV_TIMEOUT_MS 1000

#define SERIAL_LOOP_DELAY_MS 10
#define RF_LOOP_DELAY_MS 20

#define MAX_STRING_LENGTH 64

#define DEBUG_RX_PIN 21
#define DEBUG_GENERIC_PIN 1

#define DEBUG_LED_PIN 25

// Macro for printf_safe which calls printf only if the serial port is connected
#define printf_safe(...) if (tud_cdc_connected()) {printf(__VA_ARGS__);}

// Mutexes for telemetry and controller states
mutex telemetry_mutex;
mutex controller_mutex;

// Synced by LoRa and serial threads respectively
char telemetryState[MAX_STRING_LENGTH];
char controllerState[MAX_STRING_LENGTH];

/* Variables for core 0 only */
bool connected = false;
char rf_outgoing_buffer[MAX_STRING_LENGTH];
char rf_incoming_buffer[MAX_STRING_LENGTH];

/* Variables for core 1 only */
char serial_tmp_buffer[MAX_STRING_LENGTH];

// Timestamp of the last received packet
uint64_t last_packet_time = 0;
uint64_t last_send_time = 0;

bool rxPacketWaiting = false;

void onPacketRecieved(int packetSize) {
    printf("Packet Recieved\n");
    rxPacketWaiting = true;
}

void core0_rf_loop() {

    
    // Check if the connection should be marked as lost due to timeout
    if (to_ms_since_boot(get_absolute_time()) - last_packet_time >= RF_RECV_TIMEOUT_MS) {
        connected = false;
        mutex_enter_blocking(&telemetry_mutex);
        strcpy(telemetryState, "ERROR: RF Data Timeout");
        mutex_exit(&telemetry_mutex);


    }

    // Check if it's time to send the current state
    if (to_ms_since_boot(get_absolute_time()) - last_send_time >= RF_LOOP_DELAY_MS) {

       // Blip the debug LED to indicate the loop is running
        //gpio_put(DEBUG_LED_PIN, 1);
        //sleep_us(150);
        //gpio_put(DEBUG_LED_PIN, 0);
       // sleep_us(10);


        mutex_enter_blocking(&controller_mutex);
        strcpy(rf_outgoing_buffer, controllerState);
        mutex_exit(&controller_mutex);


        gpio_put(DEBUG_LED_PIN, 1);
        gpio_put(DEBUG_GENERIC_PIN, 1);

        LoRa1.beginPacket();
        LoRa1.print(rf_outgoing_buffer);
        LoRa1.endPacket();
        gpio_put(DEBUG_LED_PIN, 0);
        gpio_put(DEBUG_GENERIC_PIN, 0);

        last_send_time = to_ms_since_boot(get_absolute_time());
    }
    

    //check if dio0 is high
    if(true){
        //printf("DIO0 is high\n");

        if (rxPacketWaiting) {
            rxPacketWaiting = false;

            gpio_put(DEBUG_GENERIC_PIN, 1);
            sleep_us(2000);
            gpio_put(DEBUG_GENERIC_PIN, 0);

        }


        // Check if a packet is available to be received
        int packetSize = LoRa2.parsePacket();
        if (packetSize) {

            //pull DEBUG_RX_PIN high
            gpio_put(DEBUG_RX_PIN, 1);
            sleep_us(2000);
            gpio_put(DEBUG_RX_PIN, 0);


            int i = 0;
            while (LoRa2.available() && i < MAX_STRING_LENGTH - 1) {
                gpio_put(DEBUG_RX_PIN, 1);
                rf_incoming_buffer[i++] = LoRa2.read();
                gpio_put(DEBUG_RX_PIN, 0);
            }



            rf_incoming_buffer[i] = '\0';
            gpio_put(DEBUG_RX_PIN, 1);
            gpio_put(DEBUG_RX_PIN, 0);

            mutex_enter_blocking(&telemetry_mutex);
            strcpy(telemetryState, rf_incoming_buffer);
            mutex_exit(&telemetry_mutex);

            // Mark the connection as active and update the last packet time
            connected = true;
            last_packet_time = to_ms_since_boot(get_absolute_time());
        }

    }


    // Small delay to avoid maxing out CPU usage
    sleep_ms(1);
}


void core0_entry() {
    //ss, reset, dio0
    //LoRa1.setPins(8, 9, 7, 8);


    //OLD PINS, BASICALYL ISUE IS PINS 6 AND 7 INTERFERE WITH SPI0 PINS DUE TO PCB DESIGN ERROR
   // LoRa2.setPins(3, 4, 2, 3);

    //PLAN WILL BE TO DESOLDER ON TOP AND BOTTOM OF pcB FOR THESE PINS, AND THEN JUMP WIRE TO NEW PINS

    LoRa1.setPins(12, 11, 13, 12);
    LoRa2.setPins(7, 6, 8, 7);

    
    //LoRa1.setSPIFrequency(8E5); // should be 8E6 normally
    //LoRa2.setSPIFrequency(8E5);




    //setup DEBUG_RX_PIN as output


    if (!LoRa1.begin(438E6)) {
        mutex_enter_blocking(&telemetry_mutex);
        strcpy(telemetryState, "ERROR: Radio Hardware 1 Failure");
        mutex_exit(&telemetry_mutex);
        while (1);
    }

    
    if (!LoRa2.begin(439E6)) {
        mutex_enter_blocking(&telemetry_mutex);
        strcpy(telemetryState, "ERROR: Radio Hardware 2 Failure");
        mutex_exit(&telemetry_mutex);
        while (1);
    }


        //500khz
    LoRa1.setSignalBandwidth( 500E3 );
    LoRa2.setSignalBandwidth( 500E3 );


        //enable crc
    LoRa1.enableCrc();
    LoRa2.enableCrc();

    LoRa1.setSyncWord( 0xAF);
    LoRa2.setSyncWord( 0xAB);

    LoRa1.setTxPower(18);
    LoRa2.setTxPower(18);

    

    LoRa1.setSpreadingFactor(7);
    LoRa2.setSpreadingFactor(7);

    //LoRa2.onReceive(onPacketRecieved);

    gpio_init(DEBUG_LED_PIN);
    gpio_init(DEBUG_RX_PIN);
    gpio_init(DEBUG_GENERIC_PIN);
    gpio_set_dir(DEBUG_RX_PIN, GPIO_OUT);
    gpio_set_dir(DEBUG_LED_PIN, GPIO_OUT);
    gpio_set_dir(DEBUG_GENERIC_PIN, GPIO_OUT);

    strcpy(controllerState, "NO_LINK");

    while (true) {
        core0_rf_loop();
    }
}

void core1_serial_loop() {
    for (int i = 0; i < MAX_STRING_LENGTH; i++) {
        serial_tmp_buffer[i] = '\0';
    }

    mutex_enter_blocking(&telemetry_mutex);
    strcpy(serial_tmp_buffer, telemetryState);
    mutex_exit(&telemetry_mutex);

    printf_safe("%s\n", serial_tmp_buffer);

    long int timeout_start = to_ms_since_boot(get_absolute_time());
    long int serial_recv_pos = 0;

    for (int i = 0; i < MAX_STRING_LENGTH; i++) {
        serial_tmp_buffer[i] = '\0';
    }

    while (to_ms_since_boot(get_absolute_time()) - timeout_start < SERIAL_RECV_TIMEOUT_MS) {
        if (tud_cdc_available()) {
            char inByte = getchar();
            serial_tmp_buffer[serial_recv_pos++] = inByte;

            if (inByte == '\n' || inByte == '\r' || serial_recv_pos >= MAX_STRING_LENGTH) {
                serial_tmp_buffer[serial_recv_pos] = '\0';
                break;
            }
        }
    }

    if (serial_recv_pos == 0) {
        if (!tud_cdc_available()) {
            mutex_enter_blocking(&controller_mutex);
            strcpy(controllerState, "PICO_NO_SERIAL");
            mutex_exit(&controller_mutex);
        } else {
            printf_safe("ERROR: Serial Timeout\n");
            mutex_enter_blocking(&controller_mutex);
            strcpy(controllerState, "PICO_SERIAL_ERROR");
            mutex_exit(&controller_mutex);
        }
    } else {
        mutex_enter_blocking(&controller_mutex);
        strcpy(controllerState, serial_tmp_buffer);
        mutex_exit(&controller_mutex);
    }

    sleep_ms(SERIAL_LOOP_DELAY_MS);
}

void core1_entry() {
    while (!tud_cdc_connected()) {
        sleep_ms(10);
    }

    while (true) {
        core1_serial_loop();
    }
}

int main() {
    stdio_init_all();

    mutex_init(&controller_mutex);
    mutex_init(&telemetry_mutex);

    multicore_launch_core1(core1_entry);
    core0_entry();

    return 0;
}
