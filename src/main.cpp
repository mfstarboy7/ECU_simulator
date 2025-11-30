#include <iostream>
#include <iomanip> // For std::setprecision
#include "scheduler/Scheduler.h"
#include "sensors/SensorModule.h"
#include "engine/FuelControl.h"
#include "engine/EnginePhysics.h" // <--- NEW: Include Physics
#include "dtc/DTCManager.h"
#include "can/CANBus.h"

int main() {
    // 1. Instantiate Modules
    SensorModule sensors;
    FuelControl fuel;
    Scheduler scheduler;
    DTCManager dtc;
    CANBus canBus;
    EnginePhysics engine; // <--- NEW: The Physics Engine

    // Simulation State
    float currentLoad = 0.0f; // External load (hills, transmission)

    // --- TASK 1: Physics Simulation (10ms / 100Hz) ---
    // This runs fast to make the engine feel smooth
    scheduler.addTask([&]() {
        // A. Read Driver Input (Throttle)
        float throttle = sensors.getThrottle();

        // B. Idle Speed Control (IAC) Simulation
        // If driver foot is off (throttle < 1%) and RPM is too low,
        // the ECU opens the air valve to prevent stalling.
        if (throttle < 1.0f && engine.getRPM() < 650) {
            throttle = 6.0f; // ECU "kicks" the throttle to maintain idle
        }

        // C. Update Physics
        // dt = 0.01s (10ms)
        engine.update(throttle, currentLoad, 0.01f);

        // D. Sync Physics to Sensors
        // This effectively "spins" the sensor wheel
        sensors.setSimulatedRPM((int)engine.getRPM());

    }, 10);


    // --- TASK 2: ECU Logic & Fueling (100ms / 10Hz) ---
    scheduler.addTask([&]() {
        int rpm = sensors.getRPM();
        float throttle = sensors.getThrottle();
        float coolant = sensors.getCoolantTemp();
        
        // Calculate Fuel
        // We now calculate specific injection time (ms), and we'll use 30°C as a dummy Intake Temp for now
        float pulseWidth = fuel.calculateInjectionTime(rpm, throttle, 30.0f); // Updated from Lesson 5

        // Print Dashboard
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "RPM: " << std::setw(4) << rpm 
                  << " | Throttle: " << std::setw(4) << throttle << "%"
                  << " | Load: " << currentLoad << "Nm"
                  << " | Inj: " << pulseWidth << "ms" 
                  << "\n";

    }, 100);


    // --- TASK 3: CAN Bus Broadcast (50ms / 20Hz) ---
    scheduler.addTask([&]() {
        int rpm = sensors.getRPM();
        float throttle = sensors.getThrottle();

        CANMessage msg;
        msg.id = 0x100; 
        msg.timestamp = std::chrono::steady_clock::now();
        
        // Pack Data: [RPM High, RPM Low, Throttle, 0, 0, 0, 0, 0]
        msg.data[0] = (rpm >> 8) & 0xFF;
        msg.data[1] = rpm & 0xFF;
        msg.data[2] = (uint8_t)throttle;
        for(int i=3; i<8; i++) msg.data[i] = 0;

        canBus.sendMessage(msg);
    }, 50);


    // --- TASK 4: CAN Receiver (TCU Simulation) (100ms) ---
    scheduler.addTask([&]() {
        auto msgs = canBus.readMessages();
        for(const auto& m : msgs) {
            // Check for TCU Torque Reduction Command
            if(m.id == 0x200) {
                int torqueReq = m.data[0];
                
                // If TCU asks for low torque (< 100Nm), we simulate HIGH LOAD
                // causing the RPM to drop (simulating a gear shift drag or hill)
                if (torqueReq < 100) {
                    currentLoad = 80.0f; // Add load
                    std::cout << ">>> [ECU] TCU Requested Torque Reduction -> Applying Load!\n";
                } else {
                    currentLoad = 0.0f;  // Release load
                }
            }
        }
    }, 100);

    // --- TASK 5: Transmission Simulation (Sends cmds to ECU) (3000ms) ---
    scheduler.addTask([&]() {
        CANMessage msg;
        msg.id = 0x200;
        
        static bool toggle = false;
        toggle = !toggle;

        // Toggle between "Drive Normally" (200Nm) and "Shift" (50Nm)
        msg.data[0] = toggle ? 200 : 50; 
        msg.data[1] = 3; // Gear 3
        
        canBus.sendMessage(msg);
    }, 3000);


    std::cout << "ECU Simulation Started. Engine Initialized at 800 RPM.\n";
    scheduler.run();

    return 0;
}
















// #include <iostream>
// #include "scheduler/Scheduler.h"
// #include "sensors/SensorModule.h"
// #include "engine/FuelControl.h"
// #include "dtc/DTCManager.h"
// #include "engine/EnginePhysics.h"

// #include "can/CANBus.h"
// #include <cstring> // for memset if needed, or just array assignment

// int main() {

//     SensorModule sensors;
//     FuelControl fuel;
//     Scheduler scheduler;
//     CANBus canBus;
//     DTCManager dtc;
//     EnginePhysics engine;
//     float currentLoad = 0.0f; // From transmission/hills


//     // 10ms - Physics Simulation Loop
// scheduler.addTask([&]() {
//     // 1. Get inputs
//     float throttle = sensors.getThrottle();

//     // 2. Update Physics (Delta time = 0.01s)
//     engine.update(throttle, currentLoad, 0.01f);

//     // 3. Sync Sensor Module to Reality
//     // We need to tell the sensors what the ACTUAL RPM is now
//     // Note: You might need to add `setRPM` to SensorModule or just cheat here:
//     // For now, let's just print it or store it in a variable the ECU task reads
// }, 10);

//      // 100ms - ECU CAN Receiver Task
//     scheduler.addTask([&]() {
//         auto msgs = canBus.readMessages();
        
//         for(const auto& m : msgs) {
//             // Check for TCU Command (0x200)
//             if(m.id == 0x200) {
//                 int requestedTorque = m.data[0];
//                 int currentGear = m.data[1];
                
//                 std::cout << "\n>>> [ECU] RECEIVED COMMAND <<<\n";
//                 std::cout << "    Source: Transmission (TCU)\n";
//                 std::cout << "    Request: Reduce Torque to " << requestedTorque << " Nm\n";
//                 std::cout << "    Context: Gear " << currentGear << "\n";
//                 std::cout << ">>> [ECU] ADJUSTING STRATEGY...\n\n";
                
//                 // Here you could adjust fuel maps, ignition timing, etc. based on the command
//             }
//         }
//     }, 100);

//     // 50ms (20Hz) - CAN Broadcast Task
// scheduler.addTask([&]() {
//     // 1. Get current sensor data
//     int rpm = sensors.getRPM();
//     float throttle = sensors.getThrottle();
//     float coolant = sensors.getCoolantTemp();

//     // 2. Prepare CAN Frame for Engine Status
//     CANMessage msg;
//     msg.id = 0x100; // Arbitrary ID for "Engine Data"
//     msg.timestamp = std::chrono::steady_clock::now();

//     // 3. Pack data (Big Endian format typically used in Automotive)
//     // RPM fits in 2 bytes (0-65535)
//     msg.data[0] = (rpm >> 8) & 0xFF; // High byte
//     msg.data[1] = rpm & 0xFF;        // Low byte

//     // Throttle is 0-100%, fits in 1 byte
//     msg.data[2] = static_cast<uint8_t>(throttle);

//     // Coolant Temp (-40 to 215), fits in 1 byte with offset usually, 
//     // but let's just cast it for simplicity now.
//     msg.data[3] = static_cast<uint8_t>(coolant);

//     // Pad the rest with 0
//     msg.data[4] = 0; msg.data[5] = 0; msg.data[6] = 0; msg.data[7] = 0;

//     // 4. Send message
//     canBus.sendMessage(msg);

// }, 50); // Run every 50ms

// // 100ms - CAN Bus Sniffer (Simulates another ECU reading the data)
// scheduler.addTask([&]() {
//     auto msgs = canBus.readMessages();
//     for(const auto& m : msgs) {
//         if(m.id == 0x100) {
//             // Decode the data back
//             int receivedRPM = (m.data[0] << 8) | m.data[1];
//             int receivedThrottle = m.data[2];
//             std::cout << "[CAN SNIFFER] ID: 0x100 | RPM: " << receivedRPM 
//                       << " | Throttle: " << receivedThrottle << "%\n";
//         }
//     }
// }, 100);

// // Serialization: Converting complex objects (Engine State) into a byte stream.

// // Concurrency: One task writes to the bus, another reads from it, simulating two physical devices.

// // Protocol Design: You just defined a custom protocol: "ID 0x100, Byte 0-1 is RPM".


// // 200ms task — detect faults
// scheduler.addTask([&]() {
//     float coolant = sensors.getCoolantTemp();
//     float throttle = sensors.getThrottle();

//     if (coolant > 95)
//         dtc.addFault("P0217", "Engine Overheat");

//     if (throttle < 5)
//         dtc.addFault("P0120", "TPS Circuit Low");

// }, 200);

// scheduler.addTask([&]() {
//     const auto& faults = dtc.getActiveFaults();

//     if (!faults.empty()) {
//         std::cout << "Active DTCs:\n";
//         for (const auto& f : faults) {
//             if (f.active)
//                 std::cout << "  " << f.code << ": " << f.message << "\n";
//         }
//     }
// }, 1000);

//     // 100ms (10Hz): print ECU info
//     scheduler.addTask([&]() {
//         int rpm = sensors.getRPM();
//         float throttle = sensors.getThrottle();
//         float coolant = sensors.getCoolantTemp();
//         float fuelCmd = fuel.calculateFuel(rpm, throttle, coolant);

//         std::cout << "RPM: " << rpm
//                   << " | Throttle: " << throttle
//                   << " | Coolant: " << coolant
//                   << " | Fuel: " << fuelCmd << " mg/stroke\n";

//     }, 100);

//     // 1000ms (1Hz): slower background task
//     scheduler.addTask([&]() {
//         std::cout << "[ECU] Heartbeat OK\n";
//     }, 1000);

//     std::cout << "ECU Scheduler Started...\n";

//     // --- NEW: Simulate a Transmission Control Unit (TCU) ---
//     // This task mimics an external module sending commands TO your ECU
//     scheduler.addTask([&]() {
//         CANMessage msg;
//         msg.id = 0x200; // ID for "TCU Command"
//         msg.timestamp = std::chrono::steady_clock::now();
        
//         // Simulate varying torque demand (0 to 200 Nm)
//         static int torqueReq = 0;
//         torqueReq = (torqueReq + 10) % 200; 

//         msg.data[0] = static_cast<uint8_t>(torqueReq); // Byte 0: Torque
//         msg.data[1] = 3;                               // Byte 1: Gear (Fixed at 3rd gear for now)
        
//         // Fill rest with zeros
//         for(int i=2; i<8; i++) msg.data[i] = 0;

//         canBus.sendMessage(msg); // Send it onto the bus
        
//     }, 2000); // Run every 2000ms (2 seconds)

//     scheduler.run();

//     return 0;
// }



