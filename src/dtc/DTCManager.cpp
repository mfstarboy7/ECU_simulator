#include "DTCManager.h"
#include "../memory/FlashMemory.h" // <--- NEW

DTCManager::DTCManager() {
    // Upon startup, check Flash for old codes
    faults = FlashMemory::loadDTCs();
}

void DTCManager::addFault(const std::string& code, const std::string& message) {
    for (auto& f : faults) {
        if (f.code == code) {
            // Case A: Fault already exists, just wake it up
            if (!f.active) {
                f.active = true;
                FlashMemory::saveDTCs(faults); // <--- 3. ADD THIS LINE (Save on update)
            }
            return;
        }
    }
    
    // Case B: Brand new fault
    faults.push_back({ code, message, true });
    FlashMemory::saveDTCs(faults); // <--- 4. ADD THIS LINE (Save on new)
}

void DTCManager::clearFault(const std::string& code) {
    bool changed = false;
    for (auto& f : faults) {
        if (f.code == code && f.active) {
            f.active = false;
            changed = true;
        }
    }
    
    // <--- 5. Optional: Add this to save when you clear codes too
    if (changed) {
        FlashMemory::saveDTCs(faults); 
    }
}

const std::vector<DTC>& DTCManager::getActiveFaults() const {
    return faults;
}

// void DTCManager::addFault(const std::string& code, const std::string& message) {
//     for (auto& f : faults) {
//         if (f.code == code) {
//             f.active = true;
//             return;
//         }
//     }
//     faults.push_back({ code, message, true });
// }

// void DTCManager::clearFault(const std::string& code) {
//     for (auto& f : faults) {
//         if (f.code == code)
//             f.active = false;
//     }
// }

// const std::vector<DTC>& DTCManager::getActiveFaults() const {
//     return faults;
// }

