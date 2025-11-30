#include "FuelControl.h"

// Very simplified engine map
float FuelControl::baseFuelMap(int rpm, float throttle) const {
    // Fuel demand increases with throttle and RPM
    return (throttle / 100.0f) * (rpm / 1000.0f) * 10.0f;
}

// Enrich fuel when coolant temperature is low (cold engine)
float FuelControl::coolantCorrection(float coolantTemp) const {
    if (coolantTemp < 80.0f) {
        return 1.2f;  // 20% more fuel when cold
    }
    return 1.0f;      // Normal operation
}

float FuelControl::calculateFuel(int rpm, float throttle, float coolantTemp) const {
    float base = baseFuelMap(rpm, throttle);
    float correction = coolantCorrection(coolantTemp);
    return base * correction;
}
// A real fuel map is a 2D interpolation based on RPM & load

// Coolant correction is one of MANY correction factors:

// Air temp

// Battery voltage

// Lambda (O₂ sensor)

// Start enrichment

// etc.

// Here we model the simplest realistic behavior