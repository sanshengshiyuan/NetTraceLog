// compress 模块的基础单元测试（不依赖 gtest）
#include <iostream>
#include <string>
#include <vector>

#include "compress/zlib_compress.h"
#include "compress/zstd_compress.h"

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

static void test_zlib_basic(TestResult& result) {
    logger::compress::ZlibCompress z;
    z.ResetStream();

    const std::string input = std::string(10000, 'A') + "-hello-zlib-" + std::string(5000, 'B');
    size_t bound = z.CompressBound(input.size());
    std::vector<unsigned char> buffer(bound, 0);

    size_t out_len = z.Compress(input.data(), input.size(), buffer.data(), buffer.size());
    EXPECT_TRUE(out_len > 0);

    std::string decompressed = z.DeCompress(buffer.data(), out_len);
    EXPECT_TRUE(decompressed == input);
}

static void test_zstd_basic(TestResult& result) {
    logger::compress::ZstdCompress z;
    z.ResetStream();

    const std::string input = std::string(8000, 'X') + "-hello-zstd-" + std::string(7000, 'Y');
    size_t bound = z.CompressBound(input.size());
    std::vector<unsigned char> buffer(bound, 0);

    size_t out_len = z.Compress(input.data(), input.size(), buffer.data(), buffer.size());
    EXPECT_TRUE(out_len > 0);

    std::string decompressed = z.DeCompress(buffer.data(), out_len);
    EXPECT_TRUE(decompressed == input);
}

int main() {
    TestResult result;
    try {
        test_zlib_basic(result);
        test_zstd_basic(result);
    } catch (const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << "\n";
        return 1;
    }

    if (result.failures == 0) {
        std::cout << "All compress tests passed.\n";
    }
    return result.failures == 0 ? 0 : 1;
}









