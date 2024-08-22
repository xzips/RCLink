#ifndef RADIO_CONTROLLER_HPP
#define RADIO_CONTROLLER_HPP

#include <SPI.h>
#include "RF24.h"
#include <vector>
#include <string>

#define CE_PIN 14
#define CSN_PIN 10
#define SERIAL_DEBUG false

#define MAX_SEND_QUEUE_SIZE 3

#define RETURN_MSG_DELAY_US 5 //50

#define LED_PIN 13
#define RF_RECV_TIMEOUT_MS 25

extern uint8_t address[][6];
extern bool radioNumber;
extern unsigned long last_packet_timestamp_millis;
extern bool radioConnected;
extern uint8_t lastPipeIdx;
extern char rf_outgoing_buffer[32];
extern char rf_incoming_buffer[32];
extern bool incomingBufferProcessed;



extern std::vector<std::string> send_queue;

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
    void radio_setup(RF24 &radio);


    RadioLoopState radio_loop(RF24 &radio);
    



}

#endif