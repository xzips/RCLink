#ifndef RADIO_CONTROLLER_HPP
#define RADIO_CONTROLLER_HPP

#include <SPI.h>
#include "LoRa.h"
#include <vector>
#include <string>

#define SERIAL_DEBUG false
#define LED_PIN 13
#define RF_RECV_TIMEOUT_MS 1000
#define RF_TX_PERIOD_MS 20

extern unsigned long last_packet_timestamp_millis;
extern unsigned long last_tx_timestamp_millis;
extern bool radioConnected;
extern char rf_outgoing_buffer[32];
extern char rf_incoming_buffer[32];
extern bool incomingBufferProcessed;

namespace rcon
{
    enum class RadioLoopState : int
    {
        WAITING_FOR_CONNECTION,
        DISCONNECTED_RETRYING,
        RECIVED_SENT_DATA,
        CONNECTED_IDLE,
        HARDWARE_NOT_RESPONDING
    };

    void print_radio_loop_state(RadioLoopState state);
    void clear_outgoing_buffer();
    void clear_incoming_buffer();
    void radio_setup();  // Removed the RF24 parameter as LoRa doesn't require it
    RadioLoopState radio_loop();  // Removed the RF24 parameter as LoRa doesn't require it
}

#endif
