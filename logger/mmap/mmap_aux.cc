#include "mmap_aux.h"

#include <string.h>

namespace logger {

    MmapAux::MmapAux(fpath file_path) : file_path_(std::move(file_path)), handle_(nullptr), capacity_(0){
        size_t file_size = fs::GetFileSize(file_path_);
        size_t dst_size = std::max(file_size, kDefaultCapacity);
        Reserve(dst_size);
        Init();
    }

    void MmapAux::Resize(size_t new_size) {
        if (!Isvalid()) {
            return;
        }
        EnsureCapacity(new_size);
        Header()->size = new_size;
    }

    size_t MmapAux::Size() const {
        if (!Isvalid()) {
            return 0;
        }
        return Header()->size;
    }

    size_t MmapAux::Capacity() const {
        if (!Isvalid()) {
            return 0;
        }
        return this->capacity_;
    }

    void MmapAux::EnsureCapacity(size_t new_size) {
        size_t real_size = sizeof(MmapHeader) + new_size;
        if (real_size < capacity_) {
            return;
        }
        size_t dst_size = capacity_;
        while (dst_size < real_size) {
            dst_size += GetPageSize();
        }
        Reserve(dst_size);
    }
    uint8_t* MmapAux::Data() const {
        if (!Isvalid()) {
            return nullptr;
        }
        return static_cast<uint8_t*>(handle_) + sizeof(MmapHeader);
    }
    void MmapAux::Clear() {
        if (!Isvalid()) {
            return;
        }
        Header()->size = 0;
    }

    void MmapAux::Push(const void* data, size_t size) {
        if (!Isvalid()) {
            return;
        }
        size_t new_size = Size() + size;
        EnsureCapacity(new_size);
        memcpy(Data() + Size(), data, size);
        Header()->size = new_size;
    }

    double MmapAux::GetRatio() const {
        if (!Isvalid()) {
            return 0.0;
        }
        return static_cast<double>(Size()) / (Capacity() - sizeof(MmapHeader));
    }

    void MmapAux::Reserve(size_t new_size) {
        if (new_size <= capacity_) {
            return;
        }
        new_size = GetValidCapacity(new_size);
        if (new_size == capacity_) {
            return;
        }
        Unmap();
        TryMap(new_size);
        capacity_ = new_size;
    }


    bool MmapAux::Isvalid() const {
        MmapHeader* header = Header();
        if (!header) {
            return false;
        }

        return header->magic == MmapHeader::kmagic;
    }

    MmapAux::MmapHeader* MmapAux::Header() const {
        if (!handle_) {
            return nullptr;
        }
        if (capacity_ < sizeof(MmapHeader)) {
            return nullptr;
        }
        return static_cast<MmapHeader*>(handle_);
    }

    void MmapAux::Init() {
        MmapHeader* header = Header();
        if (!header) {
            return;
        }
        if (header->magic != MmapHeader::kmagic) {
            header->magic = MmapHeader::kmagic;
            header->size = 0;
        }
    }
}