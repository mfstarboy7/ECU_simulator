    #pragma once
    #include <vector>
    #include <functional>
    #include <chrono>

    class Scheduler {
    public:
        void addTask(std::function<void()> task, int intervalMs);
        void run();

    private:
        struct Task {
            std::function<void()> func;
            int intervalMs;
            std::chrono::steady_clock::time_point lastRun;
        };

        std::vector<Task> tasks;
    };
