#pragma once
#include <array>
#include <chrono>

struct CANMessage {
    unsigned int id;                     // Message ID (0x100, 0x200, etc)
    std::array<uint8_t, 8> data;         // 8-byte CAN payload
    std::chrono::steady_clock::time_point timestamp;
};
// Basic CAN Message Structure
// Every CAN frame has up to 8 bytes of data.

// Real CAN uses timestamps; our simulator will also store them.

// id is how other ECUs know what the message means.

// Example:

// 0x100 = RPM

// 0x101 = Coolant