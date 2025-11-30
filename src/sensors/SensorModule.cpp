#include "SensorModule.h"
#include "../filters/Filter.h"   // <-- Include filtering functions

#include <random>
#include <chrono>

LowPassFilter rpmFilter(0.15f);
LowPassFilter throttleFilter(0.20f);
LowPassFilter coolantFilter(0.10f);

SensorModule::SensorModule()
    : lastRPM(800), lastThrottle(20), lastCoolant(90) {}

float SensorModule::randFloat(float min, float max) {
    static std::mt19937 rng(
        std::chrono::system_clock::now().time_since_epoch().count()
    );
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

int SensorModule::getRPM() {
    float raw = randFloat(700, 3000);
    return rpmFilter.apply(raw);
}

float SensorModule::getThrottle() {
    float raw = randFloat(0, 100);
    return throttleFilter.apply(raw);
}

float SensorModule::getCoolantTemp() {
    float raw = randFloat(70, 105);
    return coolantFilter.apply(raw);
}