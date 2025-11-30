#pragma once
#include <vector>
#include <mutex>
#include "CANMessage.h"

class CANBus {
public:
    // Send a CAN frame onto the virtual bus
    void sendMessage(const CANMessage& msg);

    // Retrieve all messages currently on the bus
    std::vector<CANMessage> readMessages();

private:
    std::vector<CANMessage> messages;
    std::mutex busMutex;
};
// Simple in-memory CAN Bus Simulator
// Multiple ECUs can send and receive CAN frames via this bus.
// In a real system, CAN frames are broadcast to all nodes.
// Here, we just store them in a vector with thread safety.
// ✔ sendMessage()

// Adds a CAN frame into the message queue.
// This simulates an ECU broadcasting a message.

// ✔ readMessages()

// Returns the messages currently on the bus.
// We will use this later for debugging and for simulating multiple ECUs.

// ✔ mutex

// In real CAN, multiple ECUs write to the bus at the same time → here we replicate that safely