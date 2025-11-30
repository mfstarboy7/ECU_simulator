#include "TaskScheduler.h"

void TaskScheduler::addTask(std::function<void()> func, int intervalMs) {
    tasks.push_back({func, intervalMs, std::chrono::steady_clock::now()});
}

void TaskScheduler::run() {
    auto now = std::chrono::steady_clock::now();

    for (auto& task : tasks) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - task.lastRun).count();

        if (elapsed >= task.intervalMs) {
            task.callback();
            task.lastRun = now;
        }
    }
}
// ✔ Tasks are executed at specified intervals
// ✔ Uses std::function for flexible callbacks