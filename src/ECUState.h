#pragma once
#include <mutex>
#include <string>

// Simple data structure to hold the snapshot of the engine
struct ECUData {
    int rpm = 0;
    float throttle = 0.0f;
    float coolant = 0.0f;
    float load = 0.0f;
    float injectionMs = 0.0f;
    std::string activeDTC = "None";
};

// Thread-safe container
class ECUState {
public:
    void update(int rpm, float throttle, float coolant, float load, float inj, std::string dtc) {
        std::lock_guard<std::mutex> lock(m);
        data.rpm = rpm;
        data.throttle = throttle;
        data.coolant = coolant;
        data.load = load;
        data.injectionMs = inj;
        data.activeDTC = dtc;
    }

    ECUData read() {
        std::lock_guard<std::mutex> lock(m);
        return data;
    }

private:
    ECUData data;
    std::mutex m;
};