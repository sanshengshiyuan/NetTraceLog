#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <mutex>

#include "../mmap/mmap_aux.h"
#include "../compress/compress.h"
#include "../context/context.h"
#include "../encrypt/encrypt.h"
#include "../encrypt/aes_encrypt.h"
#include "../space.h"
#include "../internal_log.h"
#include "../utils/file_util.h"

#include "sink.h"

namespace logger {
namespace detail {
struct ChunkHeader {
    static constexpr uint64_t kMagic = 0x9E3779B97F4A7C15ULL; 
    uint64_t magic;
    uint64_t size;
    char pub_key[128];
    ChunkHeader():magic(kMagic), size(0) {}
};

struct ItemHeader {
    static constexpr uint64_t kMagic = 0x9E3779B9UL; 
    uint64_t magic;
    uint64_t size;
    ItemHeader(): magic(kMagic), size(0) {}
};
}
class EffectiveSink : public LogSink {
public:
    struct Conf {
        std::filesystem::path dir;  //文件目录
        std::string prefix;         //文件前缀
        std::string pub_key;        //公钥
        std::chrono::minutes interval{5};   //淘汰间隔
        megabytes single_size{4};
        megabytes total_size{100};
    };
    explicit EffectiveSink(Conf conf);

    ~EffectiveSink() override = default;
    void Log(const LogMsg& message) override;

    void SetFormatter(std::unique_ptr<Formatter> formatter) override;

    void Flush() override;
private:
    void SwapCache();

    bool NeedToFile();

    void PrepareToFile();

    void ElimateFile();

    std::filesystem::path GetFilePath();

    void WriteToCache(const void* data, uint32_t size);

    void CacheToFile();


private:
    Conf conf_; //相关的配置
    std::mutex effective_sink_mutex_; //处理过程需要的锁
    std::unique_ptr<Formatter> formatter_; //持有的格式化对象
    context::TaskRunnerTag task_runner_; //线程对象标签
    std::unique_ptr<compress::Compression> compress_; //压缩器
    std::unique_ptr<encrypt::Encryption> encrypt_; //加密器
    std::unique_ptr<MmapAux> master_cache_;  //主缓存
    std::unique_ptr<MmapAux> slave_cache_;  //从缓存
    std::filesystem::path log_file_path_;   //日志文件路径
    std::string client_pub_key_;    //持有的公钥，会存在日志文件里
    std::string compress_buf_;      //压缩文件缓冲区
    std::string encrypt_buf_;      //加密文件缓冲区
    std::atomic<bool> is_free_slave; //标志位，标志从缓存是否是空闲的
};
}