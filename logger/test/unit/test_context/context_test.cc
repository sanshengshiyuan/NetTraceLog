// 简单的 context 模块单元测试（不依赖 gtest）
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>

#include "context/context.h"

namespace {

struct TestResult {
    int failures = 0;
    void expect(bool cond, const char* expr, const char* where) {
        if (!cond) {
            std::cerr << "[FAIL] " << expr << " at " << where << "\n";
            failures++;
        }
    }
};

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#define EXPECT_TRUE(cond) result.expect((cond), #cond, __FILE__ ":" STRINGIZE(__LINE__))

} // namespace

int main() {
    TestResult result;

    try {
        // 1) 创建任务执行通道（TaskRunner）
        logger::context::TaskRunnerTag tag = NEW_TASK_RUNNER(1);

        // 2) 基础 PostTask 验证
        {
            std::atomic<int> counter{0};
            POST_TASK(tag, [&counter]() { counter.fetch_add(1); });
            WAIT_TASK_IDLE(tag);
            EXPECT_TRUE(counter.load() == 1);
        }

        // 3) 多次 PostTask 验证顺序执行（单线程 TaskRunner）
        {
            std::atomic<int> counter{0};
            const int kTasks = 100;
            for (int i = 0; i < kTasks; ++i) {
                POST_TASK(tag, [&counter]() { counter.fetch_add(1); });
            }
            WAIT_TASK_IDLE(tag);
            EXPECT_TRUE(counter.load() == kTasks);
        }

        // 4) PostTaskAndGetResult 返回值验证
        {
            auto fut = EXECUTOR->PostTaskAndGetResult(tag, []() { return 42; });
            fut->wait();
            EXPECT_TRUE(fut->get() == 42);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << "\n";
        return 1;
    }

    if (result.failures == 0) {
        std::cout << "All context tests passed.\n";
    }
    return result.failures == 0 ? 0 : 1;
}











