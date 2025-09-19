#pragma once

#include "thread_pool.h"

#include <unordered_set>

namespace logger {
namespace context {

using RepeatedTaskId = uint64_t;
using TaskRunnerTag = uint64_t;
using Task = std::function<void()>;

class ExecutorTimer {
public:
    struct Internals {
        std::chrono::time_point<std::chrono::high_resolution_clock> time_point;
        Task task;
        RepeatedTaskId repeated_id;

        bool operator<(const Internals& other) const {
            return time_point > other.time_point;
        }
    };
public:
    ExecutorTimer();
    ~ExecutorTimer();

    ExecutorTimer(const ExecutorTimer& other) = delete;
    ExecutorTimer& operator = (const ExecutorTimer& other) = delete;

    bool Start();
    void Stop();

    void PostDelayTask(Task task,const std::chrono::microseconds& delta);
    RepeatedTaskId PostRepeatedTask(Task task, const std::chrono::microseconds& delta, uint64_t repeated_num);
    void CancelRepeatedTask(RepeatedTaskId task_id);
private:

    void Run();
    RepeatedTaskId GetNextRepeatedId();
    void PostTask(Task task, std::chrono::microseconds delta, RepeatedTaskId id,uint64_t repeated_num);
    void PostRepeatedTask(Task task, const std::chrono::microseconds& delta, RepeatedTaskId id,uint64_t repeated_num);
private:
    std::priority_queue<Internals> queue_;
    std::mutex executor_mutex_;
    std::condition_variable executor_cond_;
    std::atomic<bool> is_running_;
    std::unique_ptr<ThreadPool> thread_pool_;

    std::atomic<RepeatedTaskId> repeated_task_id_;
    std::unordered_set<RepeatedTaskId> repeated_id_set_;
};
class ExecutorContext {
public:
    ExecutorContext() = default;
    ~ExecutorContext() = default;

    ExecutorContext(const ExecutorContext& other) = delete;
    ExecutorContext& operator=(const ExecutorContext& other) = delete;
    TaskRunnerTag AddTaskRunner(const TaskRunnerTag& tag);
private:
    using TaskRunner = ThreadPool;
    using TaskRunnerPtr = std::unique_ptr<TaskRunner>;
    friend class Executor;

   private:
    TaskRunner* GetTaskRunner(const TaskRunnerTag& tag);
    TaskRunnerTag GetNextTaskRunner();

private:
    std::unordered_map<TaskRunnerTag, TaskRunnerPtr> task_runner_dic_;
    std::mutex executor_context_mutex_;
};

class Executor {
public:
    Executor();
    ~Executor();

    Executor(const Executor& other) = delete;
    Executor& operator=(const Executor& other) = delete;

    TaskRunnerTag AddTaskRunner(const TaskRunnerTag& tag);

    void PostTask(const TaskRunnerTag& runner_tag, Task task);

    void CancelRepeatedTask(RepeatedTaskId& task_id);

    template <typename R, typename P>
    void PostDelayedTask(const TaskRunnerTag& runner_tag, Task task, const std::chrono::duration<R, P>& delta) {
        Task func = std::bind(&Executor::PostTask, this, runner_tag, std::move(task));
        executor_timer_->Start();
        executor_timer_->PostDelayTask(std::move(func), std::chrono::duration_cast<std::chrono::microseconds>(delta));
    }

    template <typename R, typename P>
    RepeatedTaskId PostRepeatedTask(const TaskRunnerTag& runner_tag,
                                  Task task,
                                  const std::chrono::duration<R, P>& delta,
                                  uint64_t repeat_num) {
        Task func = std::bind(&Executor::PostTask, this, runner_tag, std::move(task));
        executor_timer_->Start();
        return executor_timer_->PostRepeatedTask(std::move(func),
                                             std::chrono::duration_cast<std::chrono::microseconds>(delta), repeat_num);
    }
    
    template <typename F, typename... Args>
    auto PostTaskAndGetResult(const TaskRunnerTag& runner_tag,
                                F&& f,
                                Args&&... args) -> std::shared_ptr<std::future<std::result_of_t<F(Args...)>>> {
        ExecutorContext::TaskRunner* task_runner = executor_context_->GetTaskRunner(runner_tag);
        auto ret = task_runner->RunRetTask(std::forward<F>(f), std::forward<Args>(args)...);
        return ret;
    }




private:
    std::unique_ptr<ExecutorContext> executor_context_;
    std::unique_ptr<ExecutorTimer> executor_timer_;

};

}
}