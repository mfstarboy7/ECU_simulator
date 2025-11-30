#include "DTCManager.h"

void DTCManager::addFault(const std::string& code, const std::string& message) {
    for (auto& f : faults) {
        if (f.code == code) {
            f.active = true;
            return;
        }
    }
    faults.push_back({ code, message, true });
}

void DTCManager::clearFault(const std::string& code) {
    for (auto& f : faults) {
        if (f.code == code)
            f.active = false;
    }
}

const std::vector<DTC>& DTCManager::getActiveFaults() const {
    return faults;
}

