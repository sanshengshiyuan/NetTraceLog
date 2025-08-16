#include "zlib_compress.h"

namespace logger {
namespace compress {
//判断魔数，用于标识是否被压缩过
static bool IsCompressed(const void* input, size_t input_size) {
    if (!input) {
        return false;
    }

    if (input_size < 2) {
        return false;
    }

    uint16_t magic = *(uint16_t*)input;
    if (magic == 0x9c78 || magic == 0xda78 || magic == 0x5e78 || magic == 0x0178) {
        return true;
    }

    return false;

}

size_t ZlibCompress::Compress(const void* input, size_t input_size, void* output, size_t output_size) {
    if (!input || !output || input_size == 0) {
        return 0;
    }

    if (!compress_stream_) {
        return 0;
    }
    
    compress_stream_->next_in = (Bytef*)input;
    compress_stream_->avail_in = static_cast<uInt>(input_size);

    compress_stream_->next_out = (Bytef*)output;
    compress_stream_->avail_out = static_cast<uInt>(output_size);

    int ret = Z_OK;

    do {
        ret = deflate(compress_stream_.get(), Z_SYNC_FLUSH);
        if (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_STREAM_END) {
            return 0;
        }
    } while(ret == Z_BUF_ERROR);

    size_t out_len = output_size - compress_stream_->avail_out;
    return out_len;
}

size_t ZlibCompress::CompressBound(size_t input_size) {
    return input_size + 10;
}

std::string ZlibCompress::DeCompress(const void* data, size_t size) {
    if (!data) {
        return "";
    }

    if (IsCompressed(data, size)) {
        ResetUncompressStream();
    }
    if (!uncompress_stream_) {
        return "";
    }
    uncompress_stream_->next_in = (Bytef*)data;
    uncompress_stream_->avail_in = static_cast<uInt>(size);

    std::string output;
    while (uncompress_stream_->avail_in > 0) {
        char buffer[4096] = {0};
        uncompress_stream_->next_out = (Bytef*)buffer;
        uncompress_stream_->avail_out = sizeof(buffer);
        int ret = inflate(uncompress_stream_.get(), Z_SYNC_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            return "";
        }
        output.append(buffer, sizeof(buffer) - uncompress_stream_->avail_out);
    }

    return output;
}

void ZlibCompress::ResetStream() {
    compress_stream_ = std::unique_ptr<z_stream, ZStreamDeflateDeleter>(new z_stream());
    compress_stream_->zalloc = Z_NULL;
    compress_stream_->zfree = Z_NULL;
    compress_stream_->opaque = Z_NULL;

    int32_t ret = deflateInit2(compress_stream_.get(), Z_BEST_COMPRESSION, Z_DEFLATED,MAX_WBITS, MAX_MEM_LEVEL,Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        compress_stream_.reset();
    }
}

void ZlibCompress::ResetUncompressStream() {
    uncompress_stream_ = std::unique_ptr<z_stream, ZStreamInflateDeleter>(new z_stream());
    uncompress_stream_->zalloc = Z_NULL;
    uncompress_stream_->zfree = Z_NULL;
    uncompress_stream_->opaque = Z_NULL;
    uncompress_stream_->avail_in = 0;
    uncompress_stream_->next_in = Z_NULL;

    int32_t ret = inflateInit2(uncompress_stream_.get(), MAX_WBITS); {}
    if (ret != Z_OK) {
        uncompress_stream_.reset();
    }
}
}
}