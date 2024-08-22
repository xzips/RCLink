#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>


// Structures
struct ControllerState
{
    uint16_t LeftAileron = 0;
	uint16_t RightAileron = 0;
	uint16_t FrontWheel = 0;
	uint16_t LeftElevator = 0;
	uint16_t RightElevator = 0;
	uint16_t Rudder = 0;
	uint16_t Throttle = 0;
	bool MCUReset = false;
};

struct TelemetryState
{
	int16_t Pitch = 0;
	int16_t Roll = 0;
	int16_t Yaw = 0;
    uint16_t BatteryVoltage = 0; // in millivolts
	uint64_t remoteTimestamp = 0; // in milliseconds
};


extern ControllerState controllerState;
extern TelemetryState telemetryState;

// Macros
#define BASE64_ENCODE_OUTPUT_SIZE(n) (((n + 2) / 3) * 4)
#define BASE64_DECODE_OUTPUT_SIZE(n) (((n + 3) / 4) * 3)

// Function Prototypes
void bytes2base64(const uint8_t* data, size_t length, char* base64_output);
bool base64_to_bytes(const char* base64_input, uint8_t* output, size_t* output_length);

void encode_ControllerState(const struct ControllerState* state, char* base64_output);
bool decode_ControllerState(const char* base64_input, struct ControllerState* state);

void encode_TelemetryState(const struct TelemetryState* state, char* base64_output);
bool decode_TelemetryState(const char* base64_input, struct TelemetryState* state);


bool EncodeDecodeTest(bool silent);