#include <fstream>
#include <algorithm>

#include "effective_sink.h"

namespace logger {
    //初始化流程
    EffectiveSink::EffectiveSink(Conf conf) : conf_(std::move(conf)){
        LOG_INFO("EffectiveSink: dir={}, prefix={}, pub_key={}, interval={}, single_size={}, total_size={}",
           conf_.dir.string(), conf_.prefix, conf_.pub_key, conf_.interval.count(), conf_.single_size.count(),
           conf_.total_size.count());
        if (!std::filesystem::exists(conf_.dir)) {
            std::filesystem::create_directories(conf_.dir);
        }
        task_runner_ = NEW_TASK_RUNNER(10000);
        formatter_ = std::make_unique<DefaultFormatter>();
        //创建进程的一对密钥
        auto ecdh_key = encrypt::GenECDHKey();
        //客户端的公钥放入日志文件中
        client_pub_key_ = std::get<0>(ecdh_key);
        std::string client_pri = std::get<1>(ecdh_key);
        LOG_INFO("EffectiveSink: client pub size {}", client_pub_key_.size());
        // 私钥和服务器端的公钥（解析程序生成）生成共享密钥（将hex公钥转为二进制）
        std::string server_pub_bin = encrypt::HexKeyToBinary(conf_.pub_key);
        std::string shared_secret = encrypt::GenECDHSharedSecret(client_pri, server_pub_bin);
        //共享密钥作为对称密钥
        encrypt_ = std::make_unique<encrypt::AesEncrypt>(shared_secret);
        compress_ = std::make_unique<compress::ZlibCompress>();
        master_cache_ = std::make_unique<MmapAux>(conf_.dir / "master_cache");
        slave_cache_ = std::make_unique<MmapAux>(conf_.dir / "slave_cache");
        //如果有一个有问题，就弹出异常
        if (!master_cache_ || !slave_cache_) {
            throw std::runtime_error("EffectiveSink::EffectiveSink: create mmap failed");
        }
        //从缓存有东西
        if (!slave_cache_->Empty()) {
            is_free_slave.store(true);
            PrepareToFile();
            WAIT_TASK_IDLE(task_runner_);
        }
        if (!master_cache_->Empty()) {
            if (is_free_slave.load()) {
                is_free_slave.store(false);
                SwapCache();
            }
            PrepareToFile();
        }
        POST_REPEATED_TASK(task_runner_, [this]() { ElimateFile();}, conf_.interval , -1);
    }
    void EffectiveSink::Log(const LogMsg& message)  {
        static thread_local MemoryBuf buf;
        formatter_->Format(message, &buf);
        if (master_cache_->Empty()) {
            compress_->ResetStream();
        }
        {
            std::unique_lock<std::mutex> lock(effective_sink_mutex_);
            size_t compress_bound = compress_->CompressBound(buf.size());
            compress_buf_.resize(compress_bound);
            size_t compressed_size =
                compress_->Compress(buf.data(), buf.size(), compress_buf_.data(), compress_buf_.size());
            if (compressed_size == 0) {
                LOG_ERROR("EffectiveSink::Log: compress failed");
                return;
            }
            compress_buf_.resize(compressed_size);

            encrypt_buf_.clear();
            encrypt_->Encrypt(compress_buf_.data(), compressed_size, encrypt_buf_);
            if (encrypt_buf_.empty()) {
                LOG_ERROR("EffectiveSink::Log: encrypt failed");
                return;
            }
            WriteToCache(encrypt_buf_.data(), encrypt_buf_.size());
        }
        if (NeedToFile()) {
            if (is_free_slave.load()) {
                is_free_slave.store(false);
                SwapCache();
            }
            PrepareToFile();
        }
    }
    void EffectiveSink::Flush() {
        LOG_INFO("EffectiveSink::Flush_: start");
        PrepareToFile();
        WAIT_TASK_IDLE(task_runner_);
        if (!master_cache_->Empty()) {
            if (is_free_slave.load()) {
                is_free_slave.store(false);
                SwapCache();
            }
            PrepareToFile();
            WAIT_TASK_IDLE(task_runner_);
        }
        LOG_INFO("EffectiveSink::Flush_: end");
    }
    void EffectiveSink::SwapCache() {
        std::unique_lock<std::mutex> lock(effective_sink_mutex_);
        std::swap(master_cache_, slave_cache_);
    }
    //判断需不需要写入文件中，当超过0.8的比例就写入文件
    bool EffectiveSink::NeedToFile() {
        return master_cache_->GetRatio() > 0.8;
    }
    //写入到主缓存中
    void EffectiveSink::WriteToCache(const void* data, uint32_t size) {
        detail::ItemHeader item;
        item.size = size;
        master_cache_->Push(&item, sizeof(item));
        master_cache_->Push(data, size);
    }
    //准备去写入文件，向线程增加一个写入文件的任务
    void EffectiveSink::PrepareToFile() {
        POST_TASK(task_runner_, [this]() { CacheToFile(); });
    }
    //缓存写入文件
    void EffectiveSink::CacheToFile() {
        if (is_free_slave.load()) {
            return;
        }
        if (slave_cache_->Empty()) {
            is_free_slave.store(true);
            return;
        }
        {
            auto file_path = GetFilePath();
            detail::ChunkHeader chunk_header;
            memcpy(chunk_header.pub_key, client_pub_key_.data(), client_pub_key_.size());
            LOG_INFO("chunk size: {}", chunk_header.size);
            chunk_header.size = static_cast<uint64_t>(slave_cache_->Size());
            LOG_INFO("chunk size: {}", chunk_header.size);
            std::ofstream ofs(file_path, std::ios::binary | std::ios::app);
            ofs.write(reinterpret_cast<char*>(&chunk_header), sizeof(chunk_header));
            ofs.write(reinterpret_cast<char*>(slave_cache_->Data()), static_cast<std::streamsize>(chunk_header.size));
            ofs.close();
        }
        slave_cache_->Clear();
        is_free_slave.store(true);
    }

    std::filesystem::path EffectiveSink::GetFilePath() {
    // file pattern：{prefix}_{datetime}.log or {prefix}_{datetime}_{index}.log
        auto GetDateTimePath = [this]() -> std::filesystem::path {
            std::time_t now = std::time(nullptr);
            std::tm tm;
            LocalTime(&tm, &now);
            char time_buf[32] = {0};
            std::strftime(time_buf, sizeof(time_buf), "%Y%m%d%H%M%S", &tm);
            return (conf_.dir / (conf_.prefix + "_" + time_buf));
        };
        if (log_file_path_.empty()) {
            log_file_path_ = GetDateTimePath().string() + ".log";
        } else {
            auto file_size = fs::GetFileSize(log_file_path_);
            auto single_file = space_cast<bytes>(conf_.single_size).count();
            if (file_size > single_file) {
                auto date_time_path = GetDateTimePath();
                auto file_path = date_time_path.string() + ".log";
                if (std::filesystem::exists(file_path)) {
                    int index = 0;
                    for (auto& p : std::filesystem::directory_iterator(conf_.dir)) {
                        if (p.path().string().find(date_time_path.string()) != std::string::npos) {
                            ++index;
                        }
                    }
                    log_file_path_ =  date_time_path.string() + "_" + std::to_string(index) + ".log";
                } else {
                    log_file_path_ = file_path;
                }
            }
        }
        LOG_INFO("EffectiveSink::GetFilePath_: log_file_path={}", log_file_path_.string());
        return log_file_path_;

    }

    void EffectiveSink::ElimateFile() {
        LOG_INFO("EffectiveSink::ElimateFiles_: start");
        std::vector<std::filesystem::path> files;
        for (auto& p : std::filesystem::directory_iterator(conf_.dir)) {
            if (p.path().extension() == ".log") {
                files.push_back(p.path());
            }
        }
        std::sort(files.begin(), files.end(), [](const std::filesystem::path& lfs, const std::filesystem::path& rfs) {
            return std::filesystem::last_write_time(lfs) > std::filesystem::last_write_time(rfs);
        });
        size_t total_bytes = space_cast<bytes>(conf_.total_size).count();
        size_t used_bytes = 0;
        for (auto& file : files) {
            used_bytes += fs::GetFileSize(file);
            if (used_bytes > total_bytes) {
                LOG_INFO("EffectiveSink::ElimateFiles_: remove file={}", file.string());
                std::filesystem::remove(file);
            }
        }
    }

    void EffectiveSink::SetFormatter(std::unique_ptr<Formatter> formatter) {
        this->formatter_ = std::move(formatter);
    }
}