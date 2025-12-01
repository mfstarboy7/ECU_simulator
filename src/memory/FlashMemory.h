#pragma once
#include <vector>
#include <string>
#include "../dtc/DTC.h" // We need to know what a DTC looks like

class FlashMemory {
public:
    // Save the list of active faults to a file
    static void saveDTCs(const std::vector<DTC>& faults);

    // Load faults from the file at startup
    static std::vector<DTC> loadDTCs();

private:
    static const std::string FILENAME;
};

