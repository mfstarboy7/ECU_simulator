#pragma once

class SensorModule {
public:
    SensorModule();

    int getRPM();            // Returns the stored RPM (with noise)
    float getThrottle();     
    float getCoolantTemp();  

    // NEW: Allow the Physics Engine to update the real RPM
    void setSimulatedRPM(int rpm);

private:
    float randFloat(float min, float max);

    // Last filtered values
    float lastRPM;
    float lastThrottle;
    float lastCoolant;
};
// Simulated Sensor Module
// Provides noisy readings for RPM, Throttle Position, Coolant Temp
// NEW: Allows external setting of RPM to sync with physics engine