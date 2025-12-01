#include "FlashMemory.h"
#include <fstream>
#include <sstream>
#include <iostream>

const std::string FlashMemory::FILENAME = "ecu_nvram.txt"; // Our "Flash Chip"

void FlashMemory::saveDTCs(const std::vector<DTC>& faults) {
    std::ofstream file(FILENAME);
    if (file.is_open()) {
        for (const auto& dtc : faults) {
            // Format: CODE,MESSAGE (e.g., P0217,Engine Overheat)
            if (dtc.active) {
                file << dtc.code << "," << dtc.message << "\n";
            }
        }
        file.close();
    }
}

std::vector<DTC> FlashMemory::loadDTCs() {
    std::vector<DTC> loadedFaults;
    std::ifstream file(FILENAME);
    
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            // Parse "P0217,Engine Overheat"
            std::stringstream ss(line);
            std::string code, message;
            
            if (std::getline(ss, code, ',') && std::getline(ss, message)) {
                loadedFaults.push_back({ code, message, true }); // Mark as active
                std::cout << "[FlashMemory] Restored stored fault: " << code << "\n";
            }
        }
    }
    return loadedFaults;
}

