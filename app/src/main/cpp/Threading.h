#pragma once

#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>


std::vector<std::thread> threads;
std::queue<std::function<void()>> tasks;
std::mutex queue_mutex;
std::condition_variable condition;
bool stop_threads = false;

void worker() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [] { return stop_threads || !tasks.empty(); });
            if (stop_threads && tasks.empty()) {
                return;
            }
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}

void init_thread_pool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }
}

void add_task(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push(std::move(task));
    }
    condition.notify_one();
}

void stop_thread_pool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop_threads = true;
    }
    condition.notify_all();
    for (std::thread& thread : threads) {
        thread.join();
    }
    threads.clear();
}
