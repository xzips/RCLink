#include <SPI.h>
#include "RF24.h"

#define CE_PIN 0
#define CSN_PIN 1
#define SERIAL_DEBUG true
#define RETURN_MSG_DELAY_US 500
#define LED_PIN 13


extern uint8_t address[][6];
extern bool radioNumber;
extern bool radioConnected;
extern uint8_t lastPipeIdx;
extern char rf_outgoing_buffer[32];
extern char rf_incoming_buffer[32];


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
