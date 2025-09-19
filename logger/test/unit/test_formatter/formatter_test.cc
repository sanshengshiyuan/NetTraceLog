// formatter 模块的基础单元测试（不依赖 gtest）
#include <iostream>
#include <string>

#include "formatter/default_formatter.h"
#include "formatter/effective_formatter.h"

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

static void test_default_formatter(TestResult& result) {
    logger::DefaultFormatter fmt;
    logger::LogMsg msg({__FILE__, __LINE__, __func__}, logger::LogLevel::kInfo, "hello world");
    logger::MemoryBuf buf;
    fmt.Format(msg, &buf);
    EXPECT_TRUE(buf.find("hello world") != std::string::npos);
    EXPECT_TRUE(buf.find("[I]") != std::string::npos || buf.find(" I ") != std::string::npos);
}

static void test_effective_formatter(TestResult& result) {
    logger::EffectiveFomatter ef;
    logger::LogMsg msg({__FILE__, __LINE__, __func__}, logger::LogLevel::kWarn, "payload");
    logger::MemoryBuf buf;
    ef.Format(msg, &buf);
    // 期望 protobuf 序列化为非空字节序列
    EXPECT_TRUE(!buf.empty());
}

int main() {
    TestResult result;
    try {
        test_default_formatter(result);
        test_effective_formatter(result);
    } catch (const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << "\n";
        return 1;
    }

    if (result.failures == 0) {
        std::cout << "All formatter tests passed.\n";
    }
    return result.failures == 0 ? 0 : 1;
}


