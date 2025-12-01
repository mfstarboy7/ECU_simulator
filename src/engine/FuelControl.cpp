#include "FuelControl.h"
#include <cmath>
#include <algorithm>

float FuelControl::getVE(int rpm, float throttle) {
    // Simplified VE Table: Engines breathe best at mid-RPM and high throttle
    // Real ECUs use a 3D lookup table here.
    float rpmFactor = 1.0f - std::abs(rpm - 4000) / 4000.0f; // Peak at 4000
    return 0.75f + (0.15f * rpmFactor) + (0.10f * (throttle / 100.0f));
}

float FuelControl::calculateInjectionTime(int rpm, float throttle, float intakeTemp) {
    
    // --- 1. Decel Fuel Cut Off (DFCO) ---
    // If throttle is closed and we are moving fast, cut fuel to save gas.
    if (throttle < 1.0f && rpm > 1500) {
        currentAFR = 20.0f; // Lean (Air only)
        return 0.0f;        // 0ms injection
    }

    // --- 2. Calculate Air Mass (Ideal Gas Law simplified) ---
    // Mass = Density * Volume * VE
    float engineDisplacement = 2.0f; // 2.0 Liter engine
    float airDensity = 1.225f;       // kg/m3 at sea level (simplified)
    
    // Adjust density for temp (Cold air is denser)
    airDensity *= (298.0f / (intakeTemp + 273.15f));

    float ve = getVE(rpm, throttle);
    float airMassPerCycle = (engineDisplacement / 4) * airDensity * ve; 
    // (/4 because 4 cylinders, 1 intake stroke per 2 revs? simplified per cylinder)

    // --- 3. Target AFR Strategy ---
    float targetAFR = 14.7f; // Stoichiometric (Gasoline)
    
    // Power Enrichment: If full throttle, go rich (12.5:1) for power/cooling
    if (throttle > 80.0f) targetAFR = 12.5f;

    // --- 4. Calculate Fuel Mass ---
    float fuelMass = airMassPerCycle / targetAFR;

    // --- 5. Convert to Injector Duration ---
    // Assume injector flows 250cc/min ~ 3mg/ms
    float injectorFlowRate = 3.0f; // mg per ms
    float pulseWidth = (fuelMass * 1000.0f) / injectorFlowRate; 

    currentAFR = targetAFR;
    return pulseWidth; // in milliseconds
}





// #include "FuelControl.h"

// // Very simplified engine map
// float FuelControl::baseFuelMap(int rpm, float throttle) const {
//     // Fuel demand increases with throttle and RPM
//     return (throttle / 100.0f) * (rpm / 1000.0f) * 10.0f;
// }

// // Enrich fuel when coolant temperature is low (cold engine)
// float FuelControl::coolantCorrection(float coolantTemp) const {
//     if (coolantTemp < 80.0f) {
//         return 1.2f;  // 20% more fuel when cold
//     }
//     return 1.0f;      // Normal operation
// }

// float FuelControl::calculateFuel(int rpm, float throttle, float coolantTemp) const {
//     float base = baseFuelMap(rpm, throttle);
//     float correction = coolantCorrection(coolantTemp);
//     return base * correction;
// }
// // A real fuel map is a 2D interpolation based on RPM & load

// // Coolant correction is one of MANY correction factors:

// // Air temp

// // Battery voltage

// // Lambda (O₂ sensor)

// // Start enrichment

// // etc.

// // Here we model the simplest realistic behavior



