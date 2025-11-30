#pragma once

class FuelControl {
public:
    // Calculates fuel (mg per stroke) based on RPM, throttle, and coolant temp
    float calculateFuel(int rpm, float throttle, float coolantTemp) const;

private:
    float baseFuelMap(int rpm, float throttle) const;
    float coolantCorrection(float coolantTemp) const;
};
// calculateFuel → What the ECU calls each cycle

// baseFuelMap → Engine map (fuel demand)

// coolantCorrection → Cold start enrichment