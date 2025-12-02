# Virtual ECU Simulator 🚗💻

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Build](https://img.shields.io/badge/build-CMake-green.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)

A comprehensive, real-time simulation of an automotive **Engine Control Unit (ECU)** written in C++.
This project simulates the entire control loop of an internal combustion engine, from sensor reading and physics simulation to fuel calculation, CAN bus networking, and diagnostic monitoring.

It features a live graphical dashboard built with **Dear ImGui** to visualize engine telemetry in real-time.

## 📸 Dashboard Preview

<img width="796" height="564" alt="image" src="https://github.com/user-attachments/assets/d49aeb18-9af3-4458-8d75-38b6e6f8882f" />


## 🚀 Key Features

### 1. ⚙️ Physics-Based Engine Model

Unlike simple simulators that use random numbers, this project runs a **1D physics model**:

* Calculates **Net Torque** based on combustion force vs. internal friction and load.
* Simulates **Rotational Inertia** (crankshaft/flywheel mass) for realistic RPM rev-matching and decay.
* Implements **Idle Air Control (IAC)** logic to prevent stalling when throttle is closed.

### 2. 🧠 Control Strategy

* **Fuel Injection Logic:** Calculates injection pulse width (ms) based on Air Mass, Target AFR (Air-Fuel Ratio), and Volumetric Efficiency approximations.
* **Open Loop Control:** Includes enrichment modes for Cold Start and WOT (Wide Open Throttle).

### 3. 🔌 CAN Bus Simulation

Simulates an automotive Controller Area Network (CAN):

* **Broadcasting:** Sends RPM, Throttle, and Temp packets (ID `0x100`) at 20Hz.
* **Receiving:** Listens for simulated **TCU (Transmission Control Unit)** commands (ID `0x200`) to perform torque reduction during "gear shifts."

### 4. 🛠️ Diagnostics & Memory

* **OBD-II Style Faults:** Detects conditions like **Overheating** (Coolant > 95°C).
* **Non-Volatile Storage:** Simulates EEPROM/Flash memory by saving active DTCs (Diagnostic Trouble Codes) to a file (`ecu_nvram.txt`), preserving fault states even after a restart.

### 5. 📊 Data Logging

* Records high-frequency telemetry (20Hz) to `ecu_log.csv` for post-drive analysis in Excel/MATLAB.

### 6. 🖥️ Real-Time Dashboard (GUI)

* Built with **Dear ImGui** and **GLFW**.
* Runs on a separate thread from the ECU logic to ensure the physics engine stays deterministic (10ms tick) regardless of frame rate.
* Displays live gauges, history plots, and active fault codes.

## 🏗️ Architecture

The application uses a multi-threaded architecture to simulate the separation between hardware and HMI.

| Module | Responsibility | Refresh Rate | 
| :--- | :--- | :--- | 
| **EnginePhysics** | Calculates RPM based on torque & inertia | 100 Hz (10ms) | 
| **SensorModule** | Generates noisy sensor signals with low-pass filtering | 100 Hz (10ms) | 
| **CANBus** | Thread-safe message queue for inter-module comms | Async | 
| **ECU Logic** | Calculates Fuel, Checks Faults, Logs Data | 10 Hz / 20 Hz | 
| **GUI Thread** | Renders the ImGui Dashboard | 60 FPS (V-Sync) | 

## 🛠️ Getting Started

### Prerequisites

* **CMake** (3.20 or higher)
* **C++ Compiler** (MSVC, GCC, or Clang) supporting C++17
* *Note: GLFW and Dear ImGui dependencies are fetched automatically via CMake.*

### Build Instructions

1. **Clone the repository:**
   ```bash
   git clone [https://github.com/YourUsername/ECU_simulator.git](https://github.com/YourUsername/ECU_simulator.git)
   cd ECU_simulator

2. **Configure with CMake:**
   ```bash
   cmake -S . -B build

3. **Build the project:**
   ```bash
   cmake --build build --config Debug

4. **Run the simulator:**
   ```bash
   Windows: .\build\Debug\ECU_simulator.exe
   Linux/Mac: ./build/ECU_simulator

# 🕹️ How to Use

   1. Start the App: The engine initializes at Idle (~800 RPM).

   2. Monitor: Watch the Dashboard. The Green/Red bars indicate status.

   3. Simulate Load: Every 3 seconds, the simulated Transmission will trigger a "Shift," causing a momentary Load spike. Watch the RPM dip and recover!

   4. Trigger a Fault:

      • The simulation randomly fluctuates coolant temp.

      • If it exceeds 95°C, a DTC (P0217) will trigger.

      • The dashboard text will turn RED.

      • Restart the app: The fault will persist (loaded from "Flash Memory") until cleared.


# 👨‍💻 About the Developer

  I am an embedded systems enthusiast passionate about automotive software architecture.
  This project was built to demonstrate proficiency in:

   • **C++ System Design**: Object-oriented design with dependency injection.
   
   • **Concurrency**: Thread management (std::thread), std::mutex, and std::atomic for safe data sharing.
   
   • **Embedded Protocols**: Understanding of CAN frame packing/unpacking and arbitration concepts.
   
   • **Control Theory**: Implementation of basic open-loop control systems and physics modeling.
   
   • **Tooling**: Building custom debug tools using ImGui.

   


     
      
      
      
