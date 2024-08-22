#include "StateSync.hpp"
#include <iostream>

ControllerState controllerState;
TelemetryState telemetryState;


bool EncodeDecodeTest(bool silent)
{
    ControllerState cs;

    cs.FrontWheel = 123;
    cs.LeftElevator = 124;
    cs.RightElevator = 125;
    cs.LeftAileron = 126;
    cs.RightAileron = 127;

    cs.MCUReset = false;
    cs.Rudder = 128;

    cs.Throttle = 1500;

    char buf[512];

    encode_ControllerState(&cs, buf);

    if (!silent)
    {
        std::cout << buf << std::endl;

    }

    ControllerState cs2;

    decode_ControllerState(buf, &cs2);


    if (!silent)
    {
        std::cout << cs2.FrontWheel << std::endl;
        std::cout << cs2.LeftElevator << std::endl;
        std::cout << cs2.RightElevator << std::endl;
        std::cout << cs2.LeftAileron << std::endl;
        std::cout << cs2.RightAileron << std::endl;
        std::cout << cs2.MCUReset << std::endl;
        std::cout << cs2.Rudder << std::endl;
        std::cout << cs2.Throttle << std::endl;

    }

	if (cs.FrontWheel == cs2.FrontWheel &&
		cs.LeftElevator == cs2.LeftElevator &&
		cs.RightElevator == cs2.RightElevator &&
		cs.LeftAileron == cs2.LeftAileron &&
		cs.RightAileron == cs2.RightAileron &&
		cs.MCUReset == cs2.MCUReset &&
		cs.Rudder == cs2.Rudder &&
		cs.Throttle == cs2.Throttle)
	{
		//return true;
	}

	else {
		return false;
	}

	TelemetryState ts;
    
	ts.Pitch = 123;
	ts.Roll = 124;
	ts.Yaw = 125;
	ts.BatteryVoltage = 126;
	ts.remoteTimestamp = 124901241242;

	encode_TelemetryState(&ts, buf);

    if (!silent)
    {
        std::cout << buf << std::endl;
    }

	TelemetryState ts2;

	decode_TelemetryState(buf, &ts2);


    if (!silent)
    {
        std::cout << ts2.Pitch << std::endl;
        std::cout << ts2.Roll << std::endl;
        std::cout << ts2.Yaw << std::endl;
        std::cout << ts2.BatteryVoltage << std::endl;
		std::cout << ts2.remoteTimestamp << std::endl;
        
    }



	if (ts.Pitch == ts2.Pitch &&
		ts.Roll == ts2.Roll &&
		ts.Yaw == ts2.Yaw &&
		ts.BatteryVoltage == ts2.BatteryVoltage&&
        ts.remoteTimestamp == ts2.remoteTimestamp)
	{
		return true;
	}

	else {
		return false;
	}
    

}


// Base64 encoding table
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Helper function to get the base64 value
static uint8_t get_base64_value(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return 255; // Invalid character
}

// Convert bytes to base64
void bytes2base64(const uint8_t* data, size_t length, char* base64_output) {
    size_t i, j;
    uint8_t buffer[3];
    size_t output_index = 0;

    for (i = 0; i < length; i += 3) {
        memset(buffer, 0, 3);
        for (j = 0; j < 3 && i + j < length; j++) {
            buffer[j] = data[i + j];
        }
        base64_output[output_index++] = base64_table[(buffer[0] & 0xfc) >> 2];
        base64_output[output_index++] = base64_table[((buffer[0] & 0x03) << 4) | ((buffer[1] & 0xf0) >> 4)];
        base64_output[output_index++] = base64_table[((buffer[1] & 0x0f) << 2) | ((buffer[2] & 0xc0) >> 6)];
        base64_output[output_index++] = base64_table[buffer[2] & 0x3f];
    }

    // Add padding if necessary
    for (j = 0; j < (3 - (length % 3)) % 3; j++) {
        base64_output[output_index - 1 - j] = '=';
    }

    base64_output[output_index] = '\0';
}

// Convert base64 to bytes
bool base64_to_bytes(const char* base64_input, uint8_t* output, size_t* output_length) {
    size_t input_length = strlen(base64_input);
    size_t i, j;
    uint8_t buffer[4];

    // Input length must be a multiple of 4
    if (input_length % 4 != 0) {
        return false; // Input length is not a multiple of 4
    }

    // Adjust the length for padding characters
    size_t actual_length = input_length;
    if (base64_input[input_length - 1] == '=') actual_length--;
    if (base64_input[input_length - 2] == '=') actual_length--;

    *output_length = 0;
    for (i = 0; i < input_length; i += 4) {
        memset(buffer, 0, 4);
        for (j = 0; j < 4; j++) {
            if (base64_input[i + j] == '=') {
                buffer[j] = 0;
            }
            else {
                buffer[j] = get_base64_value(base64_input[i + j]);
                if (buffer[j] == 255) {
                    return false; // Invalid base64 character
                }
            }
        }
        output[(*output_length)++] = (buffer[0] << 2) | (buffer[1] >> 4);
        if (i + 2 < actual_length) output[(*output_length)++] = (buffer[1] << 4) | (buffer[2] >> 2);
        if (i + 3 < actual_length) output[(*output_length)++] = (buffer[2] << 6) | buffer[3];
    }

    return true;
}

// Encode ControllerState to base64
void encode_ControllerState(const struct ControllerState* state, char* base64_output) {
    uint8_t buffer[15]; // Manually calculated size based on struct fields

    // Manual serialization
    memcpy(&buffer[0], &state->LeftAileron, sizeof(state->LeftAileron));
    memcpy(&buffer[2], &state->RightAileron, sizeof(state->RightAileron));
    memcpy(&buffer[4], &state->FrontWheel, sizeof(state->FrontWheel));
    memcpy(&buffer[6], &state->LeftElevator, sizeof(state->LeftElevator));
    memcpy(&buffer[8], &state->RightElevator, sizeof(state->RightElevator));
    memcpy(&buffer[10], &state->Rudder, sizeof(state->Rudder));
    memcpy(&buffer[12], &state->Throttle, sizeof(state->Throttle));
    buffer[14] = state->MCUReset ? 1 : 0;

    bytes2base64(buffer, sizeof(buffer), base64_output);
}

// Decode base64 to ControllerState
bool decode_ControllerState(const char* base64_input, struct ControllerState* state) {
    uint8_t buffer[15]; // Manually calculated size based on struct fields
    size_t output_length = 0;

    if (!base64_to_bytes(base64_input, buffer, &output_length)) {
        return false; // Invalid base64 input
    }

    if (output_length != sizeof(buffer)) {
        return false; // Size mismatch
    }

    // Manual deserialization
    memcpy(&state->LeftAileron, &buffer[0], sizeof(state->LeftAileron));
    memcpy(&state->RightAileron, &buffer[2], sizeof(state->RightAileron));
    memcpy(&state->FrontWheel, &buffer[4], sizeof(state->FrontWheel));
    memcpy(&state->LeftElevator, &buffer[6], sizeof(state->LeftElevator));
    memcpy(&state->RightElevator, &buffer[8], sizeof(state->RightElevator));
    memcpy(&state->Rudder, &buffer[10], sizeof(state->Rudder));
    memcpy(&state->Throttle, &buffer[12], sizeof(state->Throttle));
    state->MCUReset = buffer[14] ? true : false;

    return true;
}

// Encode TelemetryState to base64
void encode_TelemetryState(const struct TelemetryState* state, char* base64_output) {
    uint8_t buffer[16]; // Manually calculated size based on struct fields

    // Manual serialization
    memcpy(&buffer[0], &state->Pitch, sizeof(state->Pitch));
    memcpy(&buffer[2], &state->Roll, sizeof(state->Roll));
    memcpy(&buffer[4], &state->Yaw, sizeof(state->Yaw));
    memcpy(&buffer[6], &state->BatteryVoltage, sizeof(state->BatteryVoltage));
	memcpy(&buffer[8], &state->remoteTimestamp, sizeof(state->remoteTimestamp));

    bytes2base64(buffer, sizeof(buffer), base64_output);
}

// Decode base64 to TelemetryState
bool decode_TelemetryState(const char* base64_input, struct TelemetryState* state) {
    uint8_t buffer[16]; // Manually calculated size based on struct fields
    size_t output_length = 0;

    if (!base64_to_bytes(base64_input, buffer, &output_length)) {
        return false; // Invalid base64 input
    }

    if (output_length != sizeof(buffer)) {
        return false; // Size mismatch
    }

    // Manual deserialization
    memcpy(&state->Pitch, &buffer[0], sizeof(state->Pitch));
    memcpy(&state->Roll, &buffer[2], sizeof(state->Roll));
    memcpy(&state->Yaw, &buffer[4], sizeof(state->Yaw));
    memcpy(&state->BatteryVoltage, &buffer[6], sizeof(state->BatteryVoltage));
	memcpy(&state->remoteTimestamp, &buffer[8], sizeof(state->remoteTimestamp));

    return true;
}
