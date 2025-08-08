#pragma once

#include "compress.h"

#include "zlib.h"

namespace logger {
namespace compress {
struct ZStreamDeflateDeleter {
    void operator()(z_stream* stream) {
        if (stream) {
            deflateEnd(stream);
            delete stream;
        }
    }
};

struct ZStreamInflateDeleter {
    void operator()(z_stream* stream) {
        if (stream) {
            inflateEnd(stream);
            delete stream;
        }
    }
};

class ZlibCompress final : public Compression {
public:
    ~ZlibCompress() override = default;

    size_t Compress(const void* input, size_t input_size, void* output, size_t output_size) override;

    size_t CompressBound(size_t input_size) override;

    std::string DeCompress(const void* data, size_t size) override;

    void ResetStream() override;

    
private:
    void ResetUncompressStream();
    
    std::unique_ptr<z_stream, ZStreamDeflateDeleter> compress_stream_;
    std::unique_ptr<z_stream, ZStreamInflateDeleter> uncompress_stream_;
};
}
}