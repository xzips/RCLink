#include "RadioController.hpp"
#include <SPI.h>
#include <Arduino.h>
#include "LoRa.h"
#include <vector>
#include "StateSync.hpp"

bool radioConnected = false;
unsigned long last_packet_timestamp_millis = millis();
char rf_outgoing_buffer[64];
char rf_incoming_buffer[64];
bool incomingBufferProcessed = false;
unsigned long last_tx_timestamp_millis = millis();

unsigned long last_packet_count_check_timestamp_millis = millis();
int packets_in_last_sec = 0;
int prev_packets_in_last_sec = 0;

int rssi;
float snr;
int freqErr;

void rcon::clear_outgoing_buffer() {
    for (int i = 0; i < 32; i++) {
        rf_outgoing_buffer[i] = 0;
    }
}

void rcon::clear_incoming_buffer() {
    for (int i = 0; i < 32; i++) {
        rf_incoming_buffer[i] = 0;
    }
}

void rcon::radio_setup() {
    if (SERIAL_DEBUG) {
        Serial.begin(115200);
        while (!Serial) { delay(5); }
    }

    SPI.begin();



    // Set pins for both LoRa modules (ss, reset, dio)
    LoRa1.setPins(9, 7, 5);
    LoRa2.setPins(8, 6, 4);

    LoRa1.setTxPower(20);
    LoRa2.setTxPower(20);



    //lora 1 is reading
    // Initialize LoRa modules
    if (!LoRa1.begin(438E6) || !LoRa2.begin(439E6)) {
        if (SERIAL_DEBUG) {
            Serial.println(F("LoRa hardware is not responding!!!"));
        }

        pinMode(LED_PIN, OUTPUT);

        while (true) {
            digitalWrite(LED_PIN, HIGH);
            delay(500);
            digitalWrite(LED_PIN, LOW);
            delay(500);
            if (SERIAL_DEBUG) {
                Serial.println(F("LoRa hardware is not responding, please reboot to retry"));
            }
        }
    }



      //500khz
    LoRa1.setSignalBandwidth( 500E3 );
    LoRa2.setSignalBandwidth( 500E3 );

    LoRa1.setSyncWord( 0xAF);
    LoRa2.setSyncWord( 0xAB);

    
    
    LoRa1.setSpreadingFactor(7);
    LoRa2.setSpreadingFactor(7);

        //enable crc
    LoRa1.enableCrc();
    LoRa2.enableCrc();



    if (SERIAL_DEBUG) {
        Serial.println(F("LoRa hardware fully initialized, proceeding"));
    }

    //set pin 2 as output
    pinMode(TX_TIMING_DEBUG_PIN, OUTPUT);

}

rcon::RadioLoopState rcon::radio_loop() {

    if (millis() - last_packet_count_check_timestamp_millis > 1000) {
        last_packet_count_check_timestamp_millis = millis();
        prev_packets_in_last_sec = packets_in_last_sec;
        packets_in_last_sec = 0;
    }

    // Check if the connection should be marked as lost due to timeout
    if (radioConnected && (millis() - last_packet_timestamp_millis > RF_RECV_TIMEOUT_MS)) {
        radioConnected = false;
        return RadioLoopState::DISCONNECTED_RETRYING;
    }




    int packetSize = LoRa1.parsePacket();
    if (packetSize) {
        
        //pull pin 2 high
        digitalWrite(TX_TIMING_DEBUG_PIN, HIGH);

        // Read the incoming packet
        size_t i = 0;
        while (LoRa1.available() && i < sizeof(rf_incoming_buffer) - 1) {
            rf_incoming_buffer[i++] = LoRa1.read();
        }
        rf_incoming_buffer[i] = '\0';

        last_packet_timestamp_millis = millis(); // Update last received packet time
        incomingBufferProcessed = false;
        radioConnected = true;

        //if (SERIAL_DEBUG) {
            Serial.print("Received: ");
            Serial.println(rf_incoming_buffer);
        //}

        packets_in_last_sec += 1; 
        rssi = LoRa1.packetRssi();
        snr = LoRa1.packetSnr();
        freqErr = LoRa1.packetFrequencyError();

        delay(2);

        //pull pin 2 low
        digitalWrite(TX_TIMING_DEBUG_PIN, LOW);




      
    }

    
  
  //last_tx_timestamp_millis and RF_TX_PERIOD_MS
    if ( (millis() - last_tx_timestamp_millis) > RF_TX_PERIOD_MS) {

        //Serial.println("Sending TX");

        last_tx_timestamp_millis = millis();

        // Prepare and send outgoing data
        clear_outgoing_buffer();

        //telemetryState.TimeSinceLastMinute = (uint16_t)(millis() % 60000UL);
        //encode_TelemetryState(&telemetryState, rf_outgoing_buffer);

        //Serial.println(millis());
        //telemetryState.TimeSinceLastMinute = 5;


        std::string outgoing = encode(telemetryState);

        //std::string outgoing = "TEST123";

        strncpy(rf_outgoing_buffer, outgoing.c_str(), 64);

        //pull pin 2 high


        //Serial.print("Sending: ");
        //Serial.println(rf_outgoing_buffer);

        LoRa2.beginPacket();
        LoRa2.print(rf_outgoing_buffer);
        LoRa2.endPacket();

        //pull pin 2 low

        if (SERIAL_DEBUG) {
            Serial.println("Successfully sent data to remote transceiver");
        }
        return RadioLoopState::RECIVED_SENT_DATA;

    }
  
  // If no packet is received, return the appropriate state
    return radioConnected ? RadioLoopState::CONNECTED_IDLE : RadioLoopState::WAITING_FOR_CONNECTION;
}

void rcon::print_radio_loop_state(RadioLoopState state) {
    switch (state) {
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
