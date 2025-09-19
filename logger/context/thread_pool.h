#pragma once

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <future>

namespace logger {
namespace context {
class ThreadPool {
public:
    ThreadPool(uint32_t count) {
        this->thread_count_.store(count);
        this->is_avialbe_.store(false);
        this->is_shutdown_.store(false);
    }
    ThreadPool(const ThreadPool& other) = delete;
    ThreadPool& operator=(const ThreadPool& other) = delete;

    ~ThreadPool() {
        Stop();
    }

    bool Start();
    void Stop();

    template <class F, class... Args>
    void RunTask(F&& f, Args&&... args) {
        if (!this->is_avialbe_.load() || this->is_shutdown_.load()) {
            return;
        }
        auto task = std::bind(std::forward<F>(f),std::forward<Args>(args)...);
        {
            std::unique_lock<std::mutex> lock(this->task_mutex_);
            this->tasks.emplace([task]() {  task(); });
        }
        this->task_cv_.notify_all();
    }

    template <class F, class... Args>
    auto RunRetTask(F&& f, Args... args) ->std::shared_ptr<std::future<std::result_of_t<F(Args...)>>>{
        if (!this->is_avialbe_.load() || this->is_shutdown_.load()) {
            return nullptr;
        }
        using return_type = std::result_of_t<F(Args...)>;
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(this->task_mutex_);
            this->tasks.emplace([task]() {(*task)();});
        }
        this->task_cv_.notify_all();
        return std::make_shared<std::future<return_type>>(std::move(res));
    }
private:
    void AddThread();
private:

    using Task = std::function<void()>;
    using ThreadPtr = std::shared_ptr<std::thread>;
    struct ThreadInfo {
        ThreadInfo() = default;
        ~ThreadInfo();
        ThreadPtr ptr{nullptr};
    };
    using ThreadInfoPtr = std::shared_ptr<ThreadInfo>;

    std::vector<ThreadInfoPtr> worker_threads_;
    std::queue<Task> tasks;

    std::mutex task_mutex_;
    std::condition_variable task_cv_;

    std::atomic<uint32_t> thread_count_;
    std::atomic<bool> is_shutdown_;
    std::atomic<bool> is_avialbe_;
};
}
}