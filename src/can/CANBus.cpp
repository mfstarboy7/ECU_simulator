#include "CANBus.h"

// Thread-safe way to put a message on the "wire"
void CANBus::sendMessage(const CANMessage& msg) {
    std::lock_guard<std::mutex> lock(busMutex);
    messages.push_back(msg);
}

// Retrieve all messages and clear the bus (simulating that they were "received")
std::vector<CANMessage> CANBus::readMessages() {
    std::lock_guard<std::mutex> lock(busMutex);
    std::vector<CANMessage> currentMessages = messages;
    messages.clear(); // Clear the bus after reading
    return currentMessages;
}
// Simple in-memory CAN Bus Simulator
// Multiple ECUs can send and receive CAN frames via this bus.
// In a real system, CAN frames are broadcast to all nodes.

// We used std::lock_guard. Since the Scheduler might run tasks on different threads (or if we add a GUI later), we don't want to read the vector while someone else is writing to it. This prevents a crash.