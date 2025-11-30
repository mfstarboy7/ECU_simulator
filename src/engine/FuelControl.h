// #pragma once

// class FuelControl {
// public:
//     // Calculates fuel (mg per stroke) based on RPM, throttle, and coolant temp
//     float calculateFuel(int rpm, float throttle, float coolantTemp) const;

// private:
//     float baseFuelMap(int rpm, float throttle) const;
//     float coolantCorrection(float coolantTemp) const;
// };
// // calculateFuel → What the ECU calls each cycle

// // baseFuelMap → Engine map (fuel demand)

// // coolantCorrection → Cold start enrichment


#pragma once

class FuelControl {
public:
    // Returns pulse width in milliseconds
    float calculateInjectionTime(int rpm, float throttle, float intakeTemp);

    float getAFR() const { return currentAFR; }

private:
    float currentAFR = 14.7f;
    
    // Volumetric Efficiency Map (How well the cylinder fills with air)
    float getVE(int rpm, float throttle);
};
// Simple Fuel Control Module
// Inputs: RPM, Throttle %, Intake Temp
// Outputs: Fuel Pulse Width (ms)
// Uses a basic VE map and cold enrichment
// Assumptions/Simplifications:
// - Constant stoichiometric AFR (14.7)
// - Simple linear VE map
// - No transient fuel corrections (acceleration enrichment, etc.)