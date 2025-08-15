#pragma once

#include "compress.h"

#include "zstd.h"

namespace logger {
namespace compress {

class ZstdCompress final : public Compression {
public:
    ZstdCompress();

    ~ZstdCompress() override;

    size_t Compress(const void* input, size_t input_size, void* output, size_t output_size) override;

    size_t CompressBound(size_t input_size) override;

    std::string DeCompress(const void* data, size_t size) override;

    void ResetStream() override;
private:
    void ResetUncompressStream();

    ZSTD_CCtx* cctx_;
    ZSTD_DCtx* dctx_;
};
}
}