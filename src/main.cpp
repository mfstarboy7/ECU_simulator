#include <iostream>
#include <iomanip>
#include <thread>
#include <atomic>

// Modules
#include "scheduler/Scheduler.h"
#include "sensors/SensorModule.h"
#include "engine/FuelControl.h"
#include "engine/EnginePhysics.h"
#include "dtc/DTCManager.h"
#include "can/CANBus.h"
#include "logging/Logger.h"
#include "ECUState.h"

// GUI Includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// --- GLOBAL SHARED STATE ---
ECUState ecuState;
std::atomic<bool> appRunning(true); // Flag to stop threads when window closes

// --- THE ECU THREAD (Background Logic) ---
void ecuTask() {
    SensorModule sensors;
    FuelControl fuel;
    Scheduler scheduler;
    DTCManager dtc;
    CANBus canBus;
    EnginePhysics engine;
    Logger logger("ecu_log.csv"); 
    
    float currentLoad = 0.0f;
    auto startTime = std::chrono::steady_clock::now();

    // TASK 1: Physics (10ms)
    scheduler.addTask([&]() {

        float throttle = sensors.getThrottle();
        if (throttle < 1.0f && engine.getRPM() < 650) throttle = 6.0f; // Anti-stall
        engine.update(throttle, currentLoad, 0.01f)
        ;
        sensors.setSimulatedRPM((int)engine.getRPM());

    }, 10);

    // TASK 2: Logic & Shared State Update (50ms)
    scheduler.addTask([&]() {
        int rpm = sensors.getRPM();
        float throttle = sensors.getThrottle();
        float coolant = sensors.getCoolantTemp();
        float inj = fuel.calculateInjectionTime(rpm, throttle, 30.0f);
        
        // Get Faults
        std::string code = "None";
        const auto& faults = dtc.getActiveFaults();
        if(!faults.empty() && faults[0].active) code = faults[0].code;

        // --- UPDATE SHARED STATE FOR GUI ---
        ecuState.update(rpm, throttle, coolant, currentLoad, inj, code);

        // Fault Logic
        if (coolant > 92.0f) dtc.addFault("P0217", "Engine Overheat");

    }, 50);

    // TASK 3: Print Active Faults to Console (1000ms) <--- ADDED THIS FOR YOU
    scheduler.addTask([&]() {
        const auto& faults = dtc.getActiveFaults();
        
        bool headerPrinted = false;
        
        for(const auto& f : faults) {
            if(f.active) {
                if(!headerPrinted) {
                    std::cout << "\n!!! ACTIVE DTCs !!!\n";
                    headerPrinted = true;
                }
                std::cout << "  CODE: " << f.code << " - " << f.message << "\n";
            }
        }
        
        if(headerPrinted) {
            std::cout << "!!!!!!!!!!!!!!!!!!!\n\n";
        }
    }, 1000);

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
                  << " | Coolant: " << coolant << "C"
                  << " | Inj: " << pulseWidth << "ms" 
                  << "\n";

    }, 100);

    // --- TASK 5: CAN Receiver (TCU Simulation) (100ms) ---
    // Reads messages from the bus. If ID 0x200 (Transmission) asks for low torque,
    // we simulate a high load on the engine.
    scheduler.addTask([&]() {
        auto msgs = canBus.readMessages();
        for(const auto& m : msgs) {
            if(m.id == 0x200) {
                int torqueReq = m.data[0];

                // DEBUG PRINT: Show we received it
                std::cout << "[CAN-RX] ID: 0x200 | TorqueReq: " << torqueReq << "Nm\n";

                if (torqueReq < 100) {
                    currentLoad = 80.0f; // Apply "Brake" load
                    std::cout << ">>> [ECU] TCU Requested Torque Reduction -> Applying Load!\n";
                } else {
                    currentLoad = 0.0f;
                }
            }
        }
    }, 100);

    // --- TASK 6: Transmission Simulation (3000ms) ---
    // Simulates an external Transmission module sending commands every 3 seconds
    scheduler.addTask([&]() {
        CANMessage msg;
        msg.id = 0x200;
        static bool toggle = false;
        toggle = !toggle;

        // Toggle between "Drive Normally" (200Nm) and "Shift" (50Nm)
        msg.data[0] = toggle ? 200 : 50; 
        msg.data[1] = 3; // Gear 3
        canBus.sendMessage(msg);

        // DEBUG PRINT: Show we sent it
        std::cout << "[TCU-TX] Sending Gear Shift Command: " << (int)msg.data[0] << "Nm\n";
    }, 3000);

    // Run until app closes
    while (appRunning) {
        // We manually tick the scheduler logic here.
        // Ideally, Scheduler should have a safe loop, but we will rely on 
        // its internal loop or run it carefully.
        // Since your Scheduler::run() is an infinite loop, we can't easily break it
        // from outside without modifying Scheduler.h.
        // For now, this thread will just die when main() exits, which is acceptable for a sim.
        scheduler.run(); 
    }
}

// --- MAIN (GUI Thread) ---
int main() {
    // 1. Start ECU in Background Thread
    std::thread ecuThread(ecuTask);
    ecuThread.detach(); // Let it run independently

    // 2. Setup Window (GLFW)
    if (!glfwInit()) return 1;
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(800, 600, "ECU Simulator Pro", nullptr, nullptr);
    if (!window) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable V-Sync

    // 3. Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // 4. GUI Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- READ DATA FROM ECU ---
        ECUData data = ecuState.read();

        // --- DRAW DASHBOARD ---
        // Make the window cover the whole application
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Dashboard", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

        // Title
        ImGui::Text("ECU SIMULATOR V1.0");
        ImGui::Separator();

        // RPM GAUGE (Progress Bar)
        float rpmFraction = data.rpm / 7000.0f;
        ImGui::Text("Engine Speed: %d RPM", data.rpm);
        // Change color based on RPM (Green -> Red)
        if(rpmFraction > 0.85f) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1, 0, 0, 1));
        else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0, 1, 0, 1));
        
        ImGui::ProgressBar(rpmFraction, ImVec2(-1, 0.0f)); // -1 width = full width
        ImGui::PopStyleColor();

        // COLUMNS
        ImGui::Columns(2, "sensor_columns");
        
        // Left Column: Sliders (Visual Only - Controls Simulation Input)
        ImGui::Text("Throttle");
        ImGui::SliderFloat("##throttle", &data.throttle, 0.0f, 100.0f, "%.1f %%");
        
        ImGui::Text("Coolant Temp");
        // Change color if overheating
        if (data.coolant > 95.0f) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.8f, 0, 0, 1));
        ImGui::SliderFloat("##coolant", &data.coolant, 0.0f, 120.0f, "%.1f C");
        if (data.coolant > 95.0f) ImGui::PopStyleColor();

        ImGui::NextColumn();

        // Right Column: Stats
        ImGui::Text("Injection: %.2f ms", data.injectionMs);
        ImGui::Text("Load:      %.1f Nm", data.load);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // DTC DISPLAY
        ImGui::Text("DIAGNOSTICS:");
        if (data.activeDTC != "None") {
            // Big Red Box for DTC
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1)); // Yellow Text
            ImGui::Text("STATUS: CRITICAL FAULT");
            ImGui::PopStyleColor();
            
            ImGui::SetWindowFontScale(2.0f); // Make text huge
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "[ %s ]", data.activeDTC.c_str());
            ImGui::SetWindowFontScale(1.0f); // Reset font
        } else {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "SYSTEM OK");
        }

        ImGui::End(); // End Dashboard Window

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    appRunning = false;
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;

}





// #include <iostream>
// #include <iomanip> // For std::setprecision
// #include "scheduler/Scheduler.h"
// #include "sensors/SensorModule.h"
// #include "engine/FuelControl.h"
// #include "engine/EnginePhysics.h" // <--- NEW: Include Physics
// #include "dtc/DTCManager.h
// #include "can/CANBus.h"
// #include "logging/Logger.h"
// #include <chrono> // For calculating time elapsed

// int main() {
//     // 1. Instantiate Modules
//     SensorModule sensors;
//     FuelControl fuel;
//     Scheduler scheduler;


//     DTCManager dtc;

//     CANBus canBus;
//     EnginePhysics engine; // <--- NEW: The Physics Engine
//     Logger logger("ecu_log.csv"); // Will create this file in your build folder 
//     auto startTime = std::chrono::steady_clock::now();



//     // Simulation State
//     float currentLoad = 0.0f; // External load (hills, transmission)

//     // --- TASK 1: Physics Simulation (10ms / 100Hz) ---
//     // This runs fast to make the engine feel smooth
//     scheduler.addTask([&]() {
//         // A. Read Driver Input (Throttle)
//         float throttle = sensors.getThrottle();

//         // B. Idle Speed Control (IAC) Simulation
//         // If driver foot is off (throttle < 1%) and RPM is too low,
//         // the ECU opens the air valve to prevent stalling.
//         if (throttle < 1.0f && engine.getRPM() < 650) {
//             throttle = 6.0f; // ECU "kicks" the throttle to maintain idle
//         }

//         // C. Update Physics
//         // dt = 0.01s (10ms)
//         engine.update(throttle, currentLoad, 0.01f);

//         // D. Sync Physics to Sensors
//         // This effectively "spins" the sensor wheel
//         sensors.setSimulatedRPM((int)engine.getRPM());

//     }, 10);


//     // --- TASK 1.5: Fault Detection (200ms) ---
//     // This looks for sensor out-of-range values and sets DTCs
//     scheduler.addTask([&]() {
//         float coolant = sensors.getCoolantTemp();
//         float throttle = sensors.getThrottle();

//         // Check for Overheat
//         if (coolant > 91.0f) {
//             // DEBUG PRINT: Confirm the logic is triggering
//             std::cout << "\n[ALERT] Overheat Detected! (Temp: " << coolant << ")\n"; 
//             dtc.addFault("P0217", "Engine Overheat");
//         }

//         // Check for TPS failure
//         if (throttle < 0.0f) { // Changed to 0.0 just for safety check
//              dtc.addFault("P0120", "TPS Circuit Low");
//         }
        
//     }, 200);

//     // --- TASK 1.6: Print Active Faults (1000ms) ---
//     scheduler.addTask([&]() {
//         const auto& faults = dtc.getActiveFaults();
//         if (!faults.empty()) {
//             std::cout << "\n!!! ACTIVE DTCs !!!\n";
//             for(const auto& f : faults) {
//                 if(f.active) 
//                     std::cout << "  CODE: " << f.code << " - " << f.message << "\n";
//             }
//             std::cout << "!!!!!!!!!!!!!!!!!!!\n\n";
//         }
//     }, 1000);


//     // --- TASK 2: ECU Logic & Fueling (100ms / 10Hz) ---
//     scheduler.addTask([&]() {
//         int rpm = sensors.getRPM();
//         float throttle = sensors.getThrottle();
//         float coolant = sensors.getCoolantTemp();
        
//         // Calculate Fuel
//         // We now calculate specific injection time (ms), and we'll use 30°C as a dummy Intake Temp for now
//         float pulseWidth = fuel.calculateInjectionTime(rpm, throttle, 30.0f); // Updated from Lesson 5

//         // Print Dashboard
//         std::cout << std::fixed << std::setprecision(1);
//         std::cout << "RPM: " << std::setw(4) << rpm 
//                   << " | Throttle: " << std::setw(4) << throttle << "%"
//                   << " | Load: " << currentLoad << "Nm"
//                   << " | Coolant: " << coolant << "C"
//                   << " | Inj: " << pulseWidth << "ms" 
//                   << "\n";

//     }, 100);


//     // --- TASK 3: CAN Bus Broadcast (50ms / 20Hz) ---
//     scheduler.addTask([&]() {
//         int rpm = sensors.getRPM();
//         float throttle = sensors.getThrottle();

//         CANMessage msg;
//         msg.id = 0x100; 
//         msg.timestamp = std::chrono::steady_clock::now();
        
//         // Pack Data: [RPM High, RPM Low, Throttle, 0, 0, 0, 0, 0]
//         msg.data[0] = (rpm >> 8) & 0xFF;
//         msg.data[1] = rpm & 0xFF;
//         msg.data[2] = (uint8_t)throttle;
//         for(int i=3; i<8; i++) msg.data[i] = 0;

//         canBus.sendMessage(msg);
//     }, 50);


//     // --- TASK 4: CAN Receiver (TCU Simulation) (100ms) ---
//     scheduler.addTask([&]() {
//         auto msgs = canBus.readMessages();
//         for(const auto& m : msgs) {
//             // Check for TCU Torque Reduction Command
//             if(m.id == 0x200) {
//                 int torqueReq = m.data[0];
                
//                 // If TCU asks for low torque (< 100Nm), we simulate HIGH LOAD
//                 // causing the RPM to drop (simulating a gear shift drag or hill)
//                 if (torqueReq < 100) {
//                     currentLoad = 80.0f; // Add load
//                     std::cout << ">>> [ECU] TCU Requested Torque Reduction -> Applying Load!\n";
//                 } else {
//                     currentLoad = 0.0f;  // Release load
//                 }
//             }
//         }
//     }, 100);

//     // --- TASK 5: Transmission Simulation (Sends cmds to ECU) (3000ms) ---
//     scheduler.addTask([&]() {
//         CANMessage msg;
//         msg.id = 0x200;
        
//         static bool toggle = false;
//         toggle = !toggle;

//         // Toggle between "Drive Normally" (200Nm) and "Shift" (50Nm)
//         msg.data[0] = toggle ? 200 : 50; 
//         msg.data[1] = 3; // Gear 3
        
//         canBus.sendMessage(msg);
//     }, 3000);

//     // --- TASK 6: Data Logging (50ms / 20Hz) ---
// scheduler.addTask([&]() {
//     // 1. Gather Data
//     int rpm = sensors.getRPM();
//     float throttle = sensors.getThrottle();
//     float coolant = sensors.getCoolantTemp();
//     float fuelMs = fuel.calculateInjectionTime(rpm, throttle, 30.0f); // Keep consistent with Task 2

//     // 2. Get DTCs (Just grab the first one for the log to keep it simple)
//     std::string dtcCode = "";
//     const auto& faults = dtc.getActiveFaults();
//     if (!faults.empty()) dtcCode = faults[0].code;

//     // 3. Calculate Time Elapsed (Seconds)
//     auto now = std::chrono::steady_clock::now();
//     std::chrono::duration<double> elapsed = now - startTime;
//     double timeSec = elapsed.count();

//     // 4. Write to CSV
//     logger.log(timeSec, rpm, throttle, coolant, currentLoad, fuelMs, dtcCode);

// }, 50);

//     std::cout << "ECU Simulation Started. Engine Initialized at 800 RPM.\n";
//     scheduler.run();

//     return 0;
// }
















// // #include <iostream>
// // #include "scheduler/Scheduler.h"
// // #include "sensors/SensorModule.h"
// // #include "engine/FuelControl.h"
// // #include "dtc/DTCManager.h"
// // #include "engine/EnginePhysics.h"

// // #include "can/CANBus.h"
// // #include <cstring> // for memset if needed, or just array assignment

// // int main() {

// //     SensorModule sensors;
// //     FuelControl fuel;
// //     Scheduler scheduler;
// //     CANBus canBus;
// //     DTCManager dtc;
// //     EnginePhysics engine;
// //     float currentLoad = 0.0f; // From transmission/hills


// //     // 10ms - Physics Simulation Loop
// // scheduler.addTask([&]() {
// //     // 1. Get inputs
// //     float throttle = sensors.getThrottle();

// //     // 2. Update Physics (Delta time = 0.01s)
// //     engine.update(throttle, currentLoad, 0.01f);

// //     // 3. Sync Sensor Module to Reality
// //     // We need to tell the sensors what the ACTUAL RPM is now
// //     // Note: You might need to add `setRPM` to SensorModule or just cheat here:
// //     // For now, let's just print it or store it in a variable the ECU task reads
// // }, 10);

// //      // 100ms - ECU CAN Receiver Task
// //     scheduler.addTask([&]() {
// //         auto msgs = canBus.readMessages();
        
// //         for(const auto& m : msgs) {
// //             // Check for TCU Command (0x200)
// //             if(m.id == 0x200) {
// //                 int requestedTorque = m.data[0];
// //                 int currentGear = m.data[1];
                
// //                 std::cout << "\n>>> [ECU] RECEIVED COMMAND <<<\n";
// //                 std::cout << "    Source: Transmission (TCU)\n";
// //                 std::cout << "    Request: Reduce Torque to " << requestedTorque << " Nm\n";
// //                 std::cout << "    Context: Gear " << currentGear << "\n";
// //                 std::cout << ">>> [ECU] ADJUSTING STRATEGY...\n\n";
                
// //                 // Here you could adjust fuel maps, ignition timing, etc. based on the command
// //             }
// //         }
// //     }, 100);

// //     // 50ms (20Hz) - CAN Broadcast Task
// // scheduler.addTask([&]() {
// //     // 1. Get current sensor data
// //     int rpm = sensors.getRPM();
// //     float throttle = sensors.getThrottle();
// //     float coolant = sensors.getCoolantTemp();

// //     // 2. Prepare CAN Frame for Engine Status
// //     CANMessage msg;
// //     msg.id = 0x100; // Arbitrary ID for "Engine Data"
// //     msg.timestamp = std::chrono::steady_clock::now();

// //     // 3. Pack data (Big Endian format typically used in Automotive)
// //     // RPM fits in 2 bytes (0-65535)
// //     msg.data[0] = (rpm >> 8) & 0xFF; // High byte
// //     msg.data[1] = rpm & 0xFF;        // Low byte

// //     // Throttle is 0-100%, fits in 1 byte
// //     msg.data[2] = static_cast<uint8_t>(throttle);

// //     // Coolant Temp (-40 to 215), fits in 1 byte with offset usually, 
// //     // but let's just cast it for simplicity now.
// //     msg.data[3] = static_cast<uint8_t>(coolant);

// //     // Pad the rest with 0
// //     msg.data[4] = 0; msg.data[5] = 0; msg.data[6] = 0; msg.data[7] = 0;

// //     // 4. Send message
// //     canBus.sendMessage(msg);

// // }, 50); // Run every 50ms

// // // 100ms - CAN Bus Sniffer (Simulates another ECU reading the data)
// // scheduler.addTask([&]() {
// //     auto msgs = canBus.readMessages();
// //     for(const auto& m : msgs) {
// //         if(m.id == 0x100) {
// //             // Decode the data back
// //             int receivedRPM = (m.data[0] << 8) | m.data[1];
// //             int receivedThrottle = m.data[2];
// //             std::cout << "[CAN SNIFFER] ID: 0x100 | RPM: " << receivedRPM 
// //                       << " | Throttle: " << receivedThrottle << "%\n";
// //         }
// //     }
// // }, 100);

// // // Serialization: Converting complex objects (Engine State) into a byte stream.

// // // Concurrency: One task writes to the bus, another reads from it, simulating two physical devices.

// // // Protocol Design: You just defined a custom protocol: "ID 0x100, Byte 0-1 is RPM".


// // // 200ms task — detect faults
// // scheduler.addTask([&]() {
// //     float coolant = sensors.getCoolantTemp();
// //     float throttle = sensors.getThrottle();

// //     if (coolant > 95)
// //         dtc.addFault("P0217", "Engine Overheat");

// //     if (throttle < 5)
// //         dtc.addFault("P0120", "TPS Circuit Low");

// // }, 200);

// // scheduler.addTask([&]() {
// //     const auto& faults = dtc.getActiveFaults();

// //     if (!faults.empty()) {
// //         std::cout << "Active DTCs:\n";
// //         for (const auto& f : faults) {
// //             if (f.active)
// //                 std::cout << "  " << f.code << ": " << f.message << "\n";
// //         }
// //     }
// // }, 1000);

// //     // 100ms (10Hz): print ECU info
// //     scheduler.addTask([&]() {
// //         int rpm = sensors.getRPM();
// //         float throttle = sensors.getThrottle();
// //         float coolant = sensors.getCoolantTemp();
// //         float fuelCmd = fuel.calculateFuel(rpm, throttle, coolant);

// //         std::cout << "RPM: " << rpm
// //                   << " | Throttle: " << throttle
// //                   << " | Coolant: " << coolant
// //                   << " | Fuel: " << fuelCmd << " mg/stroke\n";

// //     }, 100);

// //     // 1000ms (1Hz): slower background task
// //     scheduler.addTask([&]() {
// //         std::cout << "[ECU] Heartbeat OK\n";
// //     }, 1000);

// //     std::cout << "ECU Scheduler Started...\n";

// //     // --- NEW: Simulate a Transmission Control Unit (TCU) ---
// //     // This task mimics an external module sending commands TO your ECU
// //     scheduler.addTask([&]() {
// //         CANMessage msg;
// //         msg.id = 0x200; // ID for "TCU Command"
// //         msg.timestamp = std::chrono::steady_clock::now();
        
// //         // Simulate varying torque demand (0 to 200 Nm)
// //         static int torqueReq = 0;
// //         torqueReq = (torqueReq + 10) % 200; 

// //         msg.data[0] = static_cast<uint8_t>(torqueReq); // Byte 0: Torque
// //         msg.data[1] = 3;                               // Byte 1: Gear (Fixed at 3rd gear for now)
        
// //         // Fill rest with zeros
// //         for(int i=2; i<8; i++) msg.data[i] = 0;

// //         canBus.sendMessage(msg); // Send it onto the bus
        
// //     }, 2000); // Run every 2000ms (2 seconds)

// //     scheduler.run();

// //     return 0;
// // }



