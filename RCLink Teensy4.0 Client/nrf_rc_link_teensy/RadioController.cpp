#include "RadioController.hpp"
#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include <Arduino.h>
#include <vector>
#include "StateSync.hpp"


uint8_t address[][6] = { "1Tnsy", "2Pico" };
bool radioNumber = 1;
bool radioConnected = false;
uint8_t lastPipeIdx;
char rf_outgoing_buffer[32];
char rf_incoming_buffer[32];
unsigned long last_packet_timestamp_millis;


bool incomingBufferProcessed = false;

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


    radio.setChannel(118); // 2.518 GHz

    //radio.setPALevel(RF24_PA_LOW);

    radio.setPALevel(RF24_PA_HIGH, 0);// set power level high, and enable LNA

    radio.setPayloadSize(32);



    radio.openWritingPipe(address[radioNumber]);
    radio.openReadingPipe(1, address[!radioNumber]);


    radio.startListening();


    //printf_begin();
    //radio.printPrettyDetails();



    if (SERIAL_DEBUG) {
        Serial.println(F("Radio hardware fully initialized, proceeding"));
    }




}








rcon::RadioLoopState rcon::radio_loop(RF24 &radio)
{


    


    



    //check radio status before listening
    
    if (!radio.isChipConnected()) {
        if (SERIAL_DEBUG) {
            Serial.println("Radio hardware is not connected, retrying...");
        }

    

        return RadioLoopState::HARDWARE_NOT_RESPONDING;
    }

    //check timeout for packet
    if (radioConnected && (millis() - last_packet_timestamp_millis > RF_RECV_TIMEOUT_MS)) {
        radioConnected = false;
        return RadioLoopState::DISCONNECTED_RETRYING;
    }

    

    
 
    radio.startListening();
    delayMicroseconds(130);




    if (!radioConnected){
        if (radio.available(&lastPipeIdx)) {
            uint8_t bytes = radio.getPayloadSize();
            radio.read(&rf_incoming_buffer, bytes);
            last_packet_timestamp_millis = millis();

            //send return
            clear_outgoing_buffer();
            sprintf(rf_outgoing_buffer, "CONNECTION_ACK");

            radio.stopListening();
            delayMicroseconds(130);

            bool report = radio.write(&rf_outgoing_buffer, 32);


            if (report) {
                if (SERIAL_DEBUG) {
                    Serial.println("Successfully sent connection ack to remote tranciever");
                }
            }
            else {
                if (SERIAL_DEBUG) {
                    Serial.println("Connection ack transmission failed or timed out");
                }
                return RadioLoopState::DISCONNECTED_RETRYING;
            }


            radioConnected = true;

            return RadioLoopState::CONNECTED_IDLE;

        }
    }


    if (!radioConnected) return RadioLoopState::WAITING_FOR_CONNECTION;

     //listen for the first incoming message
    if (radio.available(&lastPipeIdx)) {

        uint8_t bytes = radio.getPayloadSize(); 
        radio.read(&rf_incoming_buffer, bytes); 

        last_packet_timestamp_millis = millis();

        incomingBufferProcessed = false;


        if (SERIAL_DEBUG) {
            Serial.print("Received: ");
            Serial.println(rf_incoming_buffer);
        }
    }

    else{
        return RadioLoopState::CONNECTED_IDLE;
    }


    //wait before sending a return message
    //delayMicroseconds(RETURN_MSG_DELAY_US);

    radio.stopListening();
    delayMicroseconds(130);


    /*
    here we need to fetch the next data to be streamed to the remote tranciever, perhaps we can have a long buffer of data to be sent and increment the index each time we send a message etc, should
    be done externally to this function
    */

    clear_outgoing_buffer();


/*
    if (send_queue.size() == 0) {

        sprintf(rf_outgoing_buffer, "%ld", millis());
    }
    else {
        sprintf(rf_outgoing_buffer, send_queue[0].c_str());
        send_queue.erase(send_queue.begin());
    }
*/

    telemetryState.remoteTimestamp = millis();


    encode_TelemetryState(&telemetryState, rf_outgoing_buffer);


    bool report = radio.write(&rf_outgoing_buffer, 32);

    if (report) {
        if (SERIAL_DEBUG) {
            //Serial.println("Successfully sent data to remote tranciever");
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

    //radio.startListening();
    //delayMicroseconds(130);

    

}






void rcon::print_radio_loop_state(RadioLoopState state)
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

    case RadioLoopState::HARDWARE_NOT_RESPONDING:
        if (SERIAL_DEBUG) {
            Serial.println("Radio Loop State: HARDWARE_NOT_RESPONDING");
        }
        break;

    default:
        break;
    }

}