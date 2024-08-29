#include "RadioController.hpp"
#include <SPI.h>
#include <Arduino.h>
#include "LoRa.h"
#include <vector>
#include "StateSync.hpp"

bool radioConnected = false;
unsigned long last_packet_timestamp_millis = millis();
char rf_outgoing_buffer[32];
char rf_incoming_buffer[32];
bool incomingBufferProcessed = false;
unsigned long last_tx_timestamp_millis = millis();


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

    // Initialize LoRa modules
    if (!LoRa1.begin(444E6) || !LoRa2.begin(446E6)) {
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

    if (SERIAL_DEBUG) {
        Serial.println(F("LoRa hardware fully initialized, proceeding"));
    }
}

rcon::RadioLoopState rcon::radio_loop() {
    // Check if the connection should be marked as lost due to timeout
    if (radioConnected && (millis() - last_packet_timestamp_millis > RF_RECV_TIMEOUT_MS)) {
        radioConnected = false;
        return RadioLoopState::DISCONNECTED_RETRYING;
    }

    int packetSize = LoRa1.parsePacket();
    if (packetSize) {
        // Read the incoming packet
        int i = 0;
        while (LoRa1.available() && i < sizeof(rf_incoming_buffer) - 1) {
            rf_incoming_buffer[i++] = LoRa1.read();
        }
        rf_incoming_buffer[i] = '\0';

        last_packet_timestamp_millis = millis(); // Update last received packet time
        incomingBufferProcessed = false;
        radioConnected = true;

        if (SERIAL_DEBUG) {
            Serial.print("Received: ");
            Serial.println(rf_incoming_buffer);
        }

      
    }

  //last_tx_timestamp_millis and RF_TX_PERIOD_MS
    if ( (millis() - last_tx_timestamp_millis) > RF_TX_PERIOD_MS) {

        Serial.println("Sending TX");

        last_tx_timestamp_millis = millis();

        // Prepare and send outgoing data
        clear_outgoing_buffer();
        telemetryState.remoteTimestamp = millis();
        encode_TelemetryState(&telemetryState, rf_outgoing_buffer);

        LoRa2.beginPacket();
        LoRa2.print(rf_outgoing_buffer);
        LoRa2.endPacket();

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
