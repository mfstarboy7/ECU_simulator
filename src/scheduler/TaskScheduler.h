#pragma once
#include <functional>
#include <vector>
#include <chrono>

struct ScheduledTask {
    std::function<void()> callback;
    int intervalMs;
    std::chrono::steady_clock::time_point lastRun;
};

class TaskScheduler {
public:
    void addTask(std::function<void()> func, int intervalMs);
    void run();
private:
    std::vector<ScheduledTask> tasks;
};
// ✔ Simple task scheduler
// ✔ Can schedule periodic tasks