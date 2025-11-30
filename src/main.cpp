#include <iostream>
#include "scheduler/Scheduler.h"
#include "sensors/SensorModule.h"
#include "engine/FuelControl.h"
#include "dtc/DTCManager.h"

#include "can/CANBus.h"
#include <cstring> // for memset if needed, or just array assignment

int main() {

    SensorModule sensors;
    FuelControl fuel;
    Scheduler scheduler;
    CANBus canBus;
    DTCManager dtc;


    // 50ms (20Hz) - CAN Broadcast Task
scheduler.addTask([&]() {
    // 1. Get current sensor data
    int rpm = sensors.getRPM();
    float throttle = sensors.getThrottle();
    float coolant = sensors.getCoolantTemp();

    // 2. Prepare CAN Frame for Engine Status
    CANMessage msg;
    msg.id = 0x100; // Arbitrary ID for "Engine Data"
    msg.timestamp = std::chrono::steady_clock::now();

    // 3. Pack data (Big Endian format typically used in Automotive)
    // RPM fits in 2 bytes (0-65535)
    msg.data[0] = (rpm >> 8) & 0xFF; // High byte
    msg.data[1] = rpm & 0xFF;        // Low byte

    // Throttle is 0-100%, fits in 1 byte
    msg.data[2] = static_cast<uint8_t>(throttle);

    // Coolant Temp (-40 to 215), fits in 1 byte with offset usually, 
    // but let's just cast it for simplicity now.
    msg.data[3] = static_cast<uint8_t>(coolant);

    // Pad the rest with 0
    msg.data[4] = 0; msg.data[5] = 0; msg.data[6] = 0; msg.data[7] = 0;

    // 4. Send message
    canBus.sendMessage(msg);

}, 50); // Run every 50ms

// 100ms - CAN Bus Sniffer (Simulates another ECU reading the data)
scheduler.addTask([&]() {
    auto msgs = canBus.readMessages();
    for(const auto& m : msgs) {
        if(m.id == 0x100) {
            // Decode the data back
            int receivedRPM = (m.data[0] << 8) | m.data[1];
            int receivedThrottle = m.data[2];
            std::cout << "[CAN SNIFFER] ID: 0x100 | RPM: " << receivedRPM 
                      << " | Throttle: " << receivedThrottle << "%\n";
        }
    }
}, 100);

// Serialization: Converting complex objects (Engine State) into a byte stream.

// Concurrency: One task writes to the bus, another reads from it, simulating two physical devices.

// Protocol Design: You just defined a custom protocol: "ID 0x100, Byte 0-1 is RPM".


// 200ms task — detect faults
scheduler.addTask([&]() {
    float coolant = sensors.getCoolantTemp();
    float throttle = sensors.getThrottle();

    if (coolant > 95)
        dtc.addFault("P0217", "Engine Overheat");

    if (throttle < 5)
        dtc.addFault("P0120", "TPS Circuit Low");

}, 200);

scheduler.addTask([&]() {
    const auto& faults = dtc.getActiveFaults();

    if (!faults.empty()) {
        std::cout << "Active DTCs:\n";
        for (const auto& f : faults) {
            if (f.active)
                std::cout << "  " << f.code << ": " << f.message << "\n";
        }
    }
}, 1000);

    // 100ms (10Hz): print ECU info
    scheduler.addTask([&]() {
        int rpm = sensors.getRPM();
        float throttle = sensors.getThrottle();
        float coolant = sensors.getCoolantTemp();
        float fuelCmd = fuel.calculateFuel(rpm, throttle, coolant);

        std::cout << "RPM: " << rpm
                  << " | Throttle: " << throttle
                  << " | Coolant: " << coolant
                  << " | Fuel: " << fuelCmd << " mg/stroke\n";

    }, 100);

    // 1000ms (1Hz): slower background task
    scheduler.addTask([&]() {
        std::cout << "[ECU] Heartbeat OK\n";
    }, 1000);

    std::cout << "ECU Scheduler Started...\n";
    scheduler.run();

    return 0;
}
