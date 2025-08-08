#include "zstd_compress.h"

namespace logger {
namespace compress {
static bool IsZstdCompressed(const void* input, size_t input_size) {
    if (input == nullptr || input_size < 4) {
        return false;
    }

    const uint8_t ZSTD_MAGIC[] = {0x28, 0xB5, 0x2F, 0xFD};
    const uint8_t* input_bytes = static_cast<const uint8_t*>(input);
    for (size_t i = 0; i < 4; ++i) {
        if (input_bytes[i] != ZSTD_MAGIC[i]) {
            return false;
        }
    }

    return true;

}
ZstdCompress::ZstdCompress() {
    cctx_ =  ZSTD_createCCtx();
    dctx_ = ZSTD_createDCtx();
}

ZstdCompress::~ZstdCompress() {
    if (cctx_) {
        ZSTD_freeCCtx(cctx_);
    }
    if (dctx_) {
        ZSTD_freeDCtx(dctx_);
    }
}

size_t ZstdCompress::Compress(const void* input, size_t input_size, void* output, size_t output_size) {
    if (!input || !output || input_size == 0) {
        return 0;
    }
    ZSTD_inBuffer input_buffer = {input, input_size, 0};
    ZSTD_outBuffer output_buffer = {output, output_size, 0};
    size_t ret = ZSTD_compressStream2(cctx_, &output_buffer, &input_buffer, ZSTD_e_flush);
    if (ZSTD_isError(ret) != 0) {
        return 0;
    }
    return output_buffer.pos;
    
}

size_t ZstdCompress::CompressBound(size_t input_size) {
    return ZSTD_compressBound(input_size);
}

std::string ZstdCompress::DeCompress(const void* data, size_t size) {
    if (!data || size == 0) {
        return "";
    }
    if (IsZstdCompressed(data, size)) {
        ResetUncompressStream();
    }

    std::string output;
    ZSTD_inBuffer input_buffer = {data, size, 0};
    size_t outbuffer_size = ZSTD_DStreamOutSize();
    output.resize(outbuffer_size);//分配推荐的缓冲区大小
    size_t total_compressed = 0; //记录已经解压的字节数
    while (input_buffer.pos < input_buffer.size) {
        size_t remaining = output.size() - total_compressed;
        ZSTD_outBuffer output_buffer = {reinterpret_cast<void*>(&output[total_compressed]), remaining, 0};
        size_t ret = ZSTD_decompressStream(dctx_, &output_buffer, &input_buffer);
        if (ZSTD_isError(ret) != 0) {
            return "";
        }
        total_compressed += output_buffer.pos;
        
        //如果缓冲区到了80%，翻倍扩容
        if (total_compressed >= output.size() * 0.8 && input_buffer.pos < input_buffer.size) {
            output.resize(output.size() * 2);
        }
    }
    //裁剪为原来大小
    output.resize(total_compressed);
    
    return output;
}

void ZstdCompress::ResetStream() {
    if (cctx_) {
        ZSTD_CCtx_reset(cctx_, ZSTD_reset_session_only);
    }
}

void ZstdCompress::ResetUncompressStream() {
    if (dctx_) {
        ZSTD_DCtx_reset(dctx_, ZSTD_reset_session_only);
    }
}
}
}