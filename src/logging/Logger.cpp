#include "Logger.h"
#include <iostream>

Logger::Logger(const std::string& filename) {
    file.open(filename, std::ios::out | std::ios::trunc); // 'trunc' overwrites the file every restart
    
    if (file.is_open()) {
        // Write CSV Header
        file << "Time(s),RPM,Throttle(%),Coolant(C),Load(Nm),Injection(ms),DTC\n";
    } else {
        std::cerr << "[Logger] Error: Could not open file " << filename << "\n";
    }
}

Logger::~Logger() {
    if (file.is_open()) {
        file.close();
    }
}

void Logger::log(double timestamp, int rpm, float throttle, float coolant, float load, float fuel, const std::string& activeDTC) {
    std::lock_guard<std::mutex> lock(logMutex); // Protect the file access
    
    if (file.is_open()) {
        file << timestamp << "," 
             << rpm << "," 
             << throttle << "," 
             << coolant << "," 
             << load << ","
             << fuel << ","
             << (activeDTC.empty() ? "None" : activeDTC) << "\n";
             
        // Optional: Flush to ensure data is saved immediately (slower, but safer if crash)
        // file.flush(); 
    }
}
