#include "StateSync.hpp"
#include <vector>



TelemetryState telemetryState;
ControllerState controllerState;

// Base64 encoding table
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string bytes_to_base64(const std::string& bytes) {
    size_t length = bytes.size();
    std::string base64_output;
    base64_output.reserve(BASE64_ENCODE_OUTPUT_SIZE(length));

    int val = 0, valb = -6;
    for (unsigned char c : bytes) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            base64_output.push_back(base64_table[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) base64_output.push_back(base64_table[((val << 8) >> (valb + 8)) & 0x3F]);
    while (base64_output.size() % 4) base64_output.push_back('=');
    return base64_output;
}

std::string base64_to_bytes(const std::string& base64_input) {
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[base64_table[i]] = i;

    std::string bytes;
    bytes.reserve(BASE64_DECODE_OUTPUT_SIZE(base64_input.size()));
    int val = 0, valb = -8;
    for (unsigned char c : base64_input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            bytes.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return bytes;
}

bool EncodeDecodeTest(bool silent)
{
    // Test ControllerState
    ControllerState cs;
    cs.FrontWheel = 123;
    cs.LeftElevator = 124;
    cs.RightElevator = 125;
    cs.LeftAileron = 126;
    cs.RightAileron = 127;
    cs.MCUReset = false;
    cs.Rudder = 128;
    cs.Throttle = 1500;
    cs.ControllerTimestamp = 123456789;
    cs.jitter_test_byte = 79;

    std::string encoded = encode(cs);

    if (!silent) {
        std::cout << encoded << std::endl;
    }

    ControllerState cs2;
    if (!decode(encoded, cs2)) {
        return false;
    }

    if (!silent) {
        auto printFields = [](const auto& state) {
            auto [LeftAileron, RightAileron, FrontWheel, LeftElevator, RightElevator,
                Rudder, Throttle, MCUReset, ControllerTimestamp, network_ID, jitter_test_byte] = state.get_fields();
            std::cout << LeftAileron << "\n" << RightAileron << "\n" << FrontWheel << "\n"
                << LeftElevator << "\n" << RightElevator << "\n" << Rudder << "\n"
                << Throttle << "\n" << MCUReset << "\n" << ControllerTimestamp << "\n"
                << network_ID << "\n" << (int)jitter_test_byte << std::endl;
            };
        printFields(cs2);
    }

    // Ensure all fields match
    if (cs.get_fields() != cs2.get_fields()) {
        return false;
    }

    // Test TelemetryState
    TelemetryState ts;
    ts.Pitch = 123;
    ts.Roll = 124;
    ts.Yaw = 125;
    ts.BatteryVoltage = 126;
    ts.remoteTimestamp = 124901241242;

    encoded = encode(ts);

    if (!silent) {
        std::cout << encoded << std::endl;
    }

    TelemetryState ts2;
    if (!decode(encoded, ts2)) {
        return false;
    }

    if (!silent) {
        auto printFields = [](const auto& state) {
            auto [Pitch, Roll, Yaw, BatteryVoltage, remoteTimestamp, network_ID] = state.get_fields();
            std::cout << Pitch << "\n" << Roll << "\n" << Yaw << "\n"
                << BatteryVoltage << "\n" << remoteTimestamp << "\n"
                << network_ID << std::endl;
            };
        printFields(ts2);
    }

    return ts.get_fields() == ts2.get_fields();
}
