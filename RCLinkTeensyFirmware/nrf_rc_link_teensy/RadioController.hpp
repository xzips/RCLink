#include <SPI.h>
#include "printf.h"
#include "RF24.h"

#define CE_PIN 0
#define CSN_PIN 1
#define SERIAL_DEBUG true
#define RETURN_MSG_DELAY 500


uint8_t address[][6] = { "1Tnsy", "2Pico" };
bool radioNumber = 1;
bool radioConnected = false;
uint8_t lastPipeIdx;
char rf_outgoing_buffer[32];
char rf_incoming_buffer[32];

namespace rcon
{
    enum class RadioLoopState
    {
        WAITING_FOR_CONNECTION,
        DISCONNECTED_RETRYING,
        RECIVED_SENT_DATA,
        CONNECTED_IDLE
    };

    void print_radio_loop_state(RadioLoopState state);

    void clear_outgoing_buffer();
    void clear_incoming_buffer();
    void radio_setup(RF24 &radio);
    RadioLoopState radio_loop(RF24 &radio);
    



}
