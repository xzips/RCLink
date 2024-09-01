#pragma once

#include <stdint.h>
#include <string>
#include <tuple>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

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
    uint8_t NetworkID = 0xa8;
    uint8_t JitterTestByte = 0;
    uint8_t LeftAileron = 0;
    uint8_t RightAileron = 0;
    uint8_t FrontWheel = 0;
    uint8_t LeftElevator = 0;
    uint8_t RightElevator = 0;
    uint8_t Rudder = 0;
    uint8_t Throttle = 0;
    uint8_t MCUReset = 0;

    // Get fields as a tuple for generic processing
    auto get_fields() const {
		return std::tie(NetworkID, JitterTestByte, LeftAileron,
			RightAileron, FrontWheel, LeftElevator, RightElevator,
            Rudder, Throttle, MCUReset);
        
	}

    auto get_fields() {
		return std::tie(NetworkID, JitterTestByte, LeftAileron,
			RightAileron, FrontWheel, LeftElevator, RightElevator,
            Rudder, Throttle, MCUReset);
    }
};

struct TelemetryState
{
    uint16_t NetworkID = 0xa9;
    int16_t Pitch = 0;
    int16_t Roll = 0;
    int16_t Yaw = 0;
    uint8_t BatteryVoltage = 0;
    uint16_t TimeSinceLastMinute = 0;

    // Get fields as a tuple for generic processing
    auto get_fields() const {
        return std::tie(NetworkID, Pitch, Roll, Yaw, BatteryVoltage, TimeSinceLastMinute);
    }

    auto get_fields() {
		return std::tie(NetworkID, Pitch, Roll, Yaw, BatteryVoltage, TimeSinceLastMinute);
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


template<typename... Fields>
void print_fields(const std::tuple<Fields...>& fields) {
    auto printField = [](const auto& field) {
        std::cout << field << " ";
        };
    std::apply([&](const Fields&... fields) { (printField(fields), ...); }, fields);
    std::cout << std::endl;
}

template<typename T>
void print_struct(const T& state) {
    print_fields(state.get_fields());
}



bool EncodeDecodeTest(bool silent);



extern TelemetryState telemetryState;
extern ControllerState controllerState;