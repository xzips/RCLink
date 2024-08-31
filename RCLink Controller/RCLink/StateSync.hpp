#pragma once

#include <stdint.h>
#include <string>
#include <tuple>
#include <sstream>
#include <iomanip>
#include <iostream>


#define BASE64_ENCODE_OUTPUT_SIZE(n) (((n + 2) / 3) * 4)
#define BASE64_DECODE_OUTPUT_SIZE(n) (((n + 3) / 4) * 3)




// Helper to encode/decode an individual item
template<typename T>
void serialize_item(std::ostringstream& stream, const T& item) {
    stream.write(reinterpret_cast<const char*>(&item), sizeof(T));
}

template<typename T>
void deserialize_item(std::istringstream& stream, T& item) {
    stream.read(reinterpret_cast<char*>(&item), sizeof(T));
}

// Function to convert a byte array to base64
std::string bytes_to_base64(const std::string& bytes);

// Function to convert a base64 string to byte array
std::string base64_to_bytes(const std::string& base64_input);

// Function to encode a struct to base64
template<typename... Fields>
std::string encode_to_base64(const std::tuple<Fields...>& fields) {
    std::ostringstream byteStream;
    auto serializeField = [&byteStream](const auto& field) {
        serialize_item(byteStream, field);
        };
    std::apply([&](const Fields&... fields) { (serializeField(fields), ...); }, fields);

    return bytes_to_base64(byteStream.str());
}

// Function to decode a struct from base64
template<typename... Fields>
bool decode_from_base64(const std::string& base64_input, std::tuple<Fields...>& fields) {
    std::string bytes = base64_to_bytes(base64_input);
    std::istringstream byteStream(bytes);

    auto deserializeField = [&byteStream](auto& field) {
        deserialize_item(byteStream, field);
        };
    std::apply([&](Fields&... fields) { (deserializeField(fields), ...); }, fields);

    return !byteStream.fail();
}

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
    uint64_t ControllerTimestamp = 0;
    uint16_t network_ID = 0xa87c;
    uint8_t jitter_test_byte = 0;

    // Get fields as a tuple for generic processing
    auto get_fields() const {
        return std::tie(LeftAileron, RightAileron, FrontWheel, LeftElevator, RightElevator,
            Rudder, Throttle, MCUReset, ControllerTimestamp, network_ID, jitter_test_byte);
    }

    auto get_fields() {
        return std::tie(LeftAileron, RightAileron, FrontWheel, LeftElevator, RightElevator,
            Rudder, Throttle, MCUReset, ControllerTimestamp, network_ID, jitter_test_byte);
    }
};

struct TelemetryState
{
    int16_t Pitch = 0;
    int16_t Roll = 0;
    int16_t Yaw = 0;
    uint16_t BatteryVoltage = 0;
    uint64_t remoteTimestamp = 0;
    uint16_t network_ID = 0xa87c;

    // Get fields as a tuple for generic processing
    auto get_fields() const {
        return std::tie(Pitch, Roll, Yaw, BatteryVoltage, remoteTimestamp, network_ID);
    }

    auto get_fields() {
        return std::tie(Pitch, Roll, Yaw, BatteryVoltage, remoteTimestamp, network_ID);
    }
};

// Macros for defining encoding/decoding sizes (now unused)

// Function Prototypes for encoding/decoding
template<typename T>
std::string encode(const T& state) {
    return encode_to_base64(state.get_fields());
}

template<typename T>
bool decode(const std::string& base64_input, T& state) {
    // Capture the fields as a tuple and store it in a local variable
    auto fields = state.get_fields();

    // Pass the local variable (which is now an lvalue) to decode_from_base64
    return decode_from_base64(base64_input, fields);
}


bool EncodeDecodeTest(bool silent);

extern TelemetryState telemetryState;
extern ControllerState controllerState;