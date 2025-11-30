#include <iostream>
#include <thread>

#include "scheduler/TaskScheduler.h"
#include "sensors/SensorModule.h"
#include "engine/FuelControl.h"

int main() {
    SensorModule sensors;
    FuelControl fuel;
    TaskScheduler scheduler;

    float rpm = 0;
    float throttle = 0;
    float coolant = 0;
    float fuelCmd = 0;

    // TASK 1: Sensor sampling every 10ms
    scheduler.addTask([&]() {
        rpm = sensors.getRPM();
        throttle = sensors.getThrottle();
        coolant = sensors.getCoolantTemp();
    }, 10);

    // TASK 2: Fuel calculation every 20ms
    scheduler.addTask([&]() {
        fuelCmd = fuel.calculateFuel((int)rpm, throttle, coolant);
    }, 20);

    // TASK 3: Print values every 1000ms (1 second)
    scheduler.addTask([&]() {
        std::cout << "ECU Simulation" << std::endl;
        std::cout << "RPM: " << rpm << std::endl;
        std::cout << "Throttle: " << throttle << " %" << std::endl;
        std::cout << "Coolant: " << coolant << " C" << std::endl;
        std::cout << "Fuel Command: " << fuelCmd << " mg/stroke" << std::endl;
        std::cout << "-------------------------" << std::endl;
    }, 1000);

    // MAIN LOOP (runs forever)
    while (true) {
        scheduler.run();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}
// ✔ Main loop runs the scheduler
// ✔ Tasks for sensor reading, fuel calculation, and printing status