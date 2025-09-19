#include "executor.h"

namespace logger {
namespace context {
    //ExecutorTimer的函数定义
    ExecutorTimer::ExecutorTimer() {
        thread_pool_ = std::make_unique<ThreadPool>(1);
        repeated_task_id_.store(0);
        is_running_.store(false);
    }
    ExecutorTimer::~ExecutorTimer() {
        Stop();
    }
    void ExecutorTimer::Stop() {
        is_running_.store(false);
        executor_cond_.notify_all();
        thread_pool_.reset();
    }
    bool ExecutorTimer::Start() {
        if (is_running_.load()) {
            return true;
        }
        bool ret = thread_pool_->Start();
        if (!ret) {
            return false;
        }
        is_running_.store(true);
        thread_pool_->RunTask(&ExecutorTimer::Run, this);
        return true;
    }
    void ExecutorTimer::PostDelayTask(Task task,const std::chrono::microseconds& delta) {
        Internals s;
        s.task = std::move(task);
        s.time_point = std::chrono::high_resolution_clock::now() + delta;
        {
            std::unique_lock<std::mutex> lock(executor_mutex_);
            queue_.push(s);
            executor_cond_.notify_all();
        }
    }
    RepeatedTaskId ExecutorTimer::PostRepeatedTask(Task task, const std::chrono::microseconds& delta, uint64_t repeated_num) {
        RepeatedTaskId id = GetNextRepeatedId();
        repeated_id_set_.insert(id);
        PostRepeatedTask(std::move(task) , delta, id, repeated_num);
        return id;
    }
    void ExecutorTimer::PostRepeatedTask(Task task, const std::chrono::microseconds& delta, RepeatedTaskId id,uint64_t repeated_num) {
        if (repeated_id_set_.find(id) == repeated_id_set_.end() || repeated_num == 0) {
            return;
        }
        task();
        Task func = std::bind(&ExecutorTimer::PostTask, this, std::move(task), delta, id, repeated_num - 1);
        Internals s;
        s.task = std::move(func);
        s.time_point = std::chrono::high_resolution_clock::now() + delta;
        s.repeated_id = std::move(id);
        {
            std::unique_lock<std::mutex> lock(executor_mutex_);
            queue_.push(s);
            executor_cond_.notify_all();
        }
    }
    RepeatedTaskId ExecutorTimer::GetNextRepeatedId () {
        return repeated_task_id_++;
    }
    void ExecutorTimer::CancelRepeatedTask(RepeatedTaskId task_id) {
        repeated_id_set_.erase(task_id);
    }
    void ExecutorTimer::PostTask(Task task, std::chrono::microseconds delta, RepeatedTaskId id,uint64_t repeated_num) {
        PostRepeatedTask(task, delta, id, repeated_num);
    }
    void ExecutorTimer::Run() {
        while (is_running_.load()) {
            std::unique_lock<std::mutex> lock(executor_mutex_);
            executor_cond_.wait(lock, [this]() {return queue_.empty();});
            auto s = queue_.top();
            auto diff = s.time_point - std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::microseconds>(diff).count() > 0) {
                executor_cond_.wait_for(lock, diff);
                continue;
            } else {
                queue_.pop();
                lock.unlock();
                s.task();
            }
        }
    }
    //ExecutorContext的函数定义
    TaskRunnerTag ExecutorContext::AddTaskRunner(const TaskRunnerTag& tag) {
        std::lock_guard<std::mutex> lock(executor_context_mutex_);
        TaskRunnerTag latest_tag = tag;
        while (task_runner_dic_.find(latest_tag) != task_runner_dic_.end()) {
            latest_tag = GetNextTaskRunner();
        }
        TaskRunnerPtr runner = std::make_unique<TaskRunner>(1);
        runner->Start();
        task_runner_dic_.emplace(latest_tag, std::move(runner));
        return latest_tag;
    }
    ExecutorContext::TaskRunner* ExecutorContext::GetTaskRunner(const TaskRunnerTag& tag) {
        std::lock_guard<std::mutex> lock(executor_context_mutex_);
        if (task_runner_dic_.find(tag) == task_runner_dic_.end()) {
            return nullptr;
        }
        return task_runner_dic_[tag].get();
    }
    TaskRunnerTag ExecutorContext::GetNextTaskRunner() {
        static uint64_t index = 0;
        index++;
        return index;
    }
    //Executor的函数定义
    Executor::Executor() {
        executor_context_ = std::make_unique<ExecutorContext>();
        executor_timer_ = std::make_unique<ExecutorTimer>();
    }

    Executor::~Executor() {
        executor_context_.reset();
        executor_timer_.reset();
    }

    TaskRunnerTag Executor::AddTaskRunner(const TaskRunnerTag& tag) {
        return executor_context_->AddTaskRunner(tag);
    }

    void Executor::PostTask(const TaskRunnerTag& tag, Task task) {
        ExecutorContext::TaskRunner* task_runner = executor_context_->GetTaskRunner(tag);
        task_runner->RunTask(std::move(task));
    }
    void Executor::CancelRepeatedTask(RepeatedTaskId& id) {
        executor_timer_->CancelRepeatedTask(id);
    }
}
}