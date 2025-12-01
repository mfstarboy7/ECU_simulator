#pragma once
#include <string>
#include <fstream>
#include <mutex>

class Logger {
public:
    // Open the file and write the CSV headers
    Logger(const std::string& filename);
    
    // Close the file properly
    ~Logger();
    
    // Write one row of data
    void log(double timestamp, int rpm, float throttle, float coolant, float load, float fuel, const std::string& activeDTC);

private:
    std::ofstream file;
    std::mutex logMutex; // Ensures thread safety if multiple tasks try to log
};

