#include <iostream>
#include "scheduler/Scheduler.h"
#include "sensors/SensorModule.h"
#include "engine/FuelControl.h"
#include "dtc/DTCManager.h"

int main() {

    SensorModule sensors;
    FuelControl fuel;
    Scheduler scheduler;

    DTCManager dtc;

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
