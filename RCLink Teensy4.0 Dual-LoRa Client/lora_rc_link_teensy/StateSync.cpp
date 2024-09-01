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
