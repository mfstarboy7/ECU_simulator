// We need a class that tracks the physical state of the engine. It takes "Throttle" and "Load" as inputs and produces "RPM" as output.

#pragma once
#include <algorithm>

class EnginePhysics {
public:
    EnginePhysics() : rpm(800.0f), internalFriction(10.0f) {}

    // Run this every cycle to update RPM based on physics
    void update(float throttlePct, float loadTorque, float dtSeconds) {
        // 1. Calculate Torque produced by combustion (simplified model)
        // More throttle = More torque. 
        // Peak torque curve usually modeled here, but we use linear for now.
        float combustionTorque = throttlePct * 2.5f; // Max 250Nm theoretically

        // 2. Calculate Friction (increases with RPM)
        float frictionTorque = internalFriction + (rpm * 0.02f);

        // 3. Net Torque = Combustion - Friction - External Load (Transmission/Hills)
        float netTorque = combustionTorque - frictionTorque - loadTorque;

        // 4. Newton's 2nd Law for Rotation: Torque = Inertia * AngularAccel
        // Inertia (flywheel + crank) roughly 0.2 kg*m^2
        float angularAccel = netTorque / 0.2f; 

        // 5. Integrate to get RPM
        // Convert rad/s^2 to RPM/s -> (accel * 60) / 2PI
        float rpmChange = (angularAccel * dtSeconds) * 9.549f; 
        
        rpm += rpmChange;

        // 6. Hard limits (Stall and Redline)
        if (rpm < 0) rpm = 0;
        if (rpm > 7000) rpm = 7000; // Rev limiter physics
    }

    float getRPM() const { return rpm; }
    void setRPM(float newRPM) { rpm = newRPM; } // For starter motor

private:
    float rpm;
    float internalFriction;
};

// Simple 1D Engine Physics Model
// Inputs: Throttle %, Load Torque
// Output: RPM
// Uses basic torque balance and rotational dynamics
// Assumptions/Simplifications:
// - Linear torque curve with throttle
// - Constant internal friction
// - Fixed rotational inertia
// - No transient effects like turbo lag or variable valve timing   
// This is sufficient for a basic ECU simulator without complex engine dynamics.
