#pragma once
#include <vector>
#include "DTC.h"

class DTCManager {
public:
    DTCManager(); // Constructor to load faults from FlashMemory
    void addFault(const std::string& code, const std::string& message);
    void clearFault(const std::string& code);
    const std::vector<DTC>& getActiveFaults() const;

private:
    std::vector<DTC> faults;
};

