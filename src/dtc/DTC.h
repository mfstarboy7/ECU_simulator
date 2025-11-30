#pragma once
#include <string>

struct DTC {
    std::string code;      // e.g., "P0120"
    std::string message;   // e.g., "Throttle Position Sensor Fault"
    bool active = false;
};


// Diagnostic Trouble Codes (DTC) System