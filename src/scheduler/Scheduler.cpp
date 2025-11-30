#include "Scheduler.h"
#include <thread>

void Scheduler::addTask(std::function<void()> task, int intervalMs) {
    tasks.push_back({task, intervalMs, std::chrono::steady_clock::now()});
}

void Scheduler::run() {
    while (true) {

        auto now = std::chrono::steady_clock::now();

        for (auto& t : tasks) {

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - t.lastRun
            ).count();

            if (elapsed >= t.intervalMs) {
                t.func();               // Run the task
                t.lastRun = now;        // Reset timer
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
