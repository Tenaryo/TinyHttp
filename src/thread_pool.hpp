#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace tinyhttp {

class ThreadPool {
  public:
    explicit ThreadPool(std::size_t num_threads) {
        workers_.reserve(num_threads);
        for (std::size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::move_only_function<void()> task;
                    {
                        std::unique_lock lock(mutex_);
                        cv_.wait(lock, [this] { return !tasks_.empty() || stop_; });
                        if (stop_ && tasks_.empty())
                            return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::lock_guard lock(mutex_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& worker : workers_)
            worker.join();
    }

    void enqueue(std::move_only_function<void()> task) {
        {
            std::lock_guard lock(mutex_);
            tasks_.push(std::move(task));
        }
        cv_.notify_one();
    }
  private:
    std::vector<std::thread> workers_;
    std::queue<std::move_only_function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_{false};
};

} // namespace tinyhttp
