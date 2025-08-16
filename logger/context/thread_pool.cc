#include "thread_pool.h"

namespace logger {
namespace context {
    bool ThreadPool::Start() {
        if (this->is_avialbe_.load()) {
            return false;
        }
        this->is_avialbe_.store(true);
        uint32_t thread_count = this->thread_count_.load();
        for (uint32_t i = 0; i < thread_count; ++i) {
            AddThread();
        }
        return true;
    }
    void ThreadPool::Stop() {
        if (is_avialbe_.load()) {
            is_shutdown_.store(true);
            task_cv_.notify_all();
            is_avialbe_.store(false);
        }
        this->worker_threads_.clear();
    }
    ThreadPool::ThreadInfo::~ThreadInfo() {
        if (ptr && ptr->joinable()) {
            ptr->join();
        }
    }
    void ThreadPool::AddThread() {
        ThreadInfoPtr thread_ptr = std::make_shared<ThreadInfo>();
        auto func = [this]() {
            for (;;) {
                Task task;
                {
                    std::unique_lock<std::mutex> lock(this->task_mutex_);
                    this->task_cv_.wait(lock, [this]() { return this->is_shutdown_ || (!this->tasks.empty());});
                    if (this->is_shutdown_) {
                        break;
                    }
                    task = this->tasks.front();
                    this->tasks.pop();
                }
                task();
            }
            
        };
        thread_ptr->ptr = std::make_shared<std::thread>(std::move(func));
        this->worker_threads_.emplace_back(std::move(thread_ptr));
        
    }
}
}