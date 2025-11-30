#pragma once

class LowPassFilter {
public:
    LowPassFilter(float alpha = 0.1f)
        : alpha(alpha), initialized(false), lastValue(0.0f) {}

    float apply(float input) {
        if (!initialized) {
            initialized = true;
            lastValue = input;
            return input;
        }

        lastValue = lastValue + alpha * (input - lastValue);
        return lastValue;
    }

private:
    float alpha;
    float lastValue;
    bool initialized;
};
// ✔ Simple low-pass filter class
// ✔ Configurable smoothing factor (alpha)