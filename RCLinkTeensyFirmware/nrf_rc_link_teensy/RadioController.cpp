#include "RadioController.hpp"
#include <SPI.h>
#include "printf.h"
#include "RF24.h"


void rcon::clear_outgoing_buffer()
{
    for (int i = 0; i < 32; i++) {
        rf_outgoing_buffer[i] = 0;
    }

}

void rcon::clear_incoming_buffer()
{
    for (int i = 0; i < 32; i++) {
        rf_incoming_buffer[i] = 0;
    }

}

void rcon::radio_setup(RF24 &radio)
{
    
    if (SERIAL_DEBUG) {
        Serial.begin(115200);
        while (!Serial) {delay(5);}
    }


    // initialize the transceiver on the SPI bus
    if (!radio.begin()) {
        if (SERIAL_DEBUG) {
            Serial.println(F("Radio hardware is not responding!!!"));
        }

        pinMode(LED_PIN, OUTPUT);
        while (true) {
            digitalWrite(LED_PIN, HIGH); 
            delay(500);           
            digitalWrite(LED_PIN, LOW); 
            delay(500);
            if (SERIAL_DEBUG) {
                Serial.println(F("Radio hardware is not responding, please reboot to retry"));   
            }         
        } 

    }



    radio.setPALevel(RF24_PA_LOW);


    radio.setPayloadSize(32);
    radio.openWritingPipe(address[radioNumber]);
    radio.openReadingPipe(1, address[!radioNumber]);


    radio.startListening();


    printf_begin();
    radio.printPrettyDetails();

    pinMode(LED_PIN, OUTPUT);

    //flash 
    /*
    for (int i = 0; i < 6; i++)
    {
        digitalWrite(LED_PIN, HIGH); 
        delay(50);           
        digitalWrite(LED_PIN, LOW); 
        delay(50);
    }
    */

    if (SERIAL_DEBUG) {
        Serial.println(F("Radio hardware fully initialized, proceeding..."));
    }

}

rcon::radio_loop(RF24 &radio)
{
    radio.startListening();
    delayMicroseconds(130);

    if (!radioConnected){
        if (radio.available(&lastPipeIdx)) {
            uint8_t bytes = radio.getPayloadSize();
            radio.read(&rf_incoming_buffer, bytes);

            radioConnected = true;
            
            if (SERIAL_DEBUG) {
                Serial.println("Connected to remote tranciever");
            }

            return RadioLoopState::CONNECTED_IDLE;

        }
    }

    if (!radioConnected) return RadioLoopState::WAITING_FOR_CONNECTION;

     //listen for the first incoming message
    if (radio.available(&lastPipeIdx)) {
        uint8_t bytes = radio.getPayloadSize(); 
        radio.read(&rf_incoming_buffer, bytes); 

        if (SERIAL_DEBUG) {
            Serial.print("Received: ");
            Serial.println(rf_incoming_buffer);
        }

        //clear the incoming buffer
        clear_incoming_buffer();



    }

    else{
        return RadioLoopState::CONNECTED_IDLE;
    }


    //wait before sending a return message
    delay(RETURN_MSG_DELAY);

    radio.stopListening();
    delayMicroseconds(130);


    /*
    here we need to fetch the next data to be streamed to the remote tranciever, perhaps we can have a long buffer of data to be sent and increment the index each time we send a message etc, should
    be done externally to this function
    */

    clear_outgoing_buffer();

    //send a return message, this would in theory be telemtry and other information data, for now print the internal clock
    sprintf(rf_outgoing_buffer, "%ld", millis());

    bool report = radio.write(&rf_outgoing_buffer, 32);

    if (report) {
        if (SERIAL_DEBUG) {
            Serial.println("Successfully sent data to remote tranciever");
        }
        return RadioLoopState::RECIVED_SENT_DATA;
    }
    else {

        if (SERIAL_DEBUG) {
            Serial.println("Data transmission failed or timed out");
        }

        radioConnected = false;
        return RadioLoopState::DISCONNECTED_RETRYING;
    }

    

}

rcon::print_radio_loop_state(RadioLoopState state)
{
    switch (state)
    {
    case RadioLoopState::WAITING_FOR_CONNECTION:
        if (SERIAL_DEBUG) {
            Serial.println("Radio Loop State: WAITING_FOR_CONNECTION");
        }
        break;
    case RadioLoopState::DISCONNECTED_RETRYING:
        if (SERIAL_DEBUG) {
            Serial.println("Radio Loop State: DISCONNECTED_RETRYING");
        }
        break;
    case RadioLoopState::RECIVED_SENT_DATA:
        if (SERIAL_DEBUG) {
            Serial.println("Radio Loop State: RECIVED_SENT_DATA");
        }
        break;

    case RadioLoopState::CONNECTED_IDLE:
        if (SERIAL_DEBUG) {
            Serial.println("Radio Loop State: CONNECTED_IDLE");
        }
        break;

    default:
        break;
    }

}