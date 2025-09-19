#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <cstring>

#include "mmap/mmap_aux.h"

namespace fs = std::filesystem;

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

#define EXPECT_TRUE(cond) result.expect((cond), #cond, __FILE__ ":" STRINGIZE(__LINE__))
#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

} // namespace

int main() {
    TestResult result;

    try {
        // 1) 生成临时测试文件路径
        fs::path tmpDir = fs::temp_directory_path();
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        fs::path filePath = tmpDir / (std::string("logger_mmap_test_") + std::to_string(now) + ".bin");

        // 确保不存在残留
        std::error_code ec;
        fs::remove(filePath, ec);

        // 2) 构造 MmapAux
        logger::MmapAux mmap(filePath);

        // 初始应为空
        EXPECT_TRUE(mmap.Size() == 0);
        EXPECT_TRUE(mmap.Empty());

        // 3) Push 写入并校验 Size 与数据内容
        const char* msg = "hello_world";
        const size_t msgLen = std::strlen(msg);
        mmap.Push(msg, msgLen);
        EXPECT_TRUE(mmap.Size() == msgLen);
        {
            const uint8_t* p = mmap.Data();
            EXPECT_TRUE(p != nullptr);
            if (p) {
                EXPECT_TRUE(std::memcmp(p, msg, msgLen) == 0);
            }
        }

        // 4) Resize 到更小
        mmap.Resize(5);
        EXPECT_TRUE(mmap.Size() == 5);
        {
            const uint8_t* p = mmap.Data();
            EXPECT_TRUE(std::memcmp(p, "hello", 5) == 0);
        }

        // 5) Clear 清空
        mmap.Clear();
        EXPECT_TRUE(mmap.Size() == 0);
        EXPECT_TRUE(mmap.Empty());

        // 6) 写入更大的数据，验证不崩溃且 Size 正确
        std::vector<uint8_t> big(100000, 0xAB);
        mmap.Push(big.data(), big.size());
        EXPECT_TRUE(mmap.Size() == big.size());
        {
            const uint8_t* p = mmap.Data();
            EXPECT_TRUE(p != nullptr);
            if (p) {
                EXPECT_TRUE(std::memcmp(p, big.data(), big.size()) == 0);
            }
        }

        // 7) GetRatio 合理（>0 且 <=1）
        double ratio = mmap.GetRatio();
        EXPECT_TRUE(ratio > 0.0);
        EXPECT_TRUE(ratio <= 1.0);

        // 清理测试文件（映射解除后文件才会被释放；这里尽力而为）
        fs::remove(filePath, ec);
    } catch (const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << "\n";
        return 1;
    }

    if (result.failures == 0) {
        std::cout << "All mmap tests passed.\n";
    }
    return result.failures == 0 ? 0 : 1;
}


