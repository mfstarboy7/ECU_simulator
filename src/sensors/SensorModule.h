#pragma once

class SensorModule {
public:
    SensorModule();

    int getRPM();            // returns filtered RPM
    float getThrottle();     // returns filtered throttle position %
    float getCoolantTemp();  // returns filtered coolant temperature °C

private:
    float randFloat(float min, float max);

    // Last filtered values (for low-pass filtering memory)
    float lastRPM;
    float lastThrottle;
    float lastCoolant;
};
// ✔ Each sensor has its own realistic noise
// ✔ Low-pass filtering smooths out rapid changes