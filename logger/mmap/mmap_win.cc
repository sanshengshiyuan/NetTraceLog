#include "mmap_aux.h"

#include <windows.h>

namespace logger {

class FileHandleDeleter {
public:
  void operator()(HANDLE h) {
    if (h != nullptr) {
      CloseHandle(h);
    }
  }
};
using FileHandlePtr = std::unique_ptr<std::remove_pointer_t<HANDLE>, FileHandleDeleter>;

bool MmapAux::TryMap(size_t capacity) {
  FileHandlePtr file_handle(CreateFileW(file_path_.wstring().c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                                        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));
  if (file_handle.get() == INVALID_HANDLE_VALUE) {
    return false;
  }
  FileHandlePtr file_mapping_handle;
  file_mapping_handle.reset(CreateFileMapping(file_handle.get(), nullptr, PAGE_READWRITE, 0, capacity, nullptr));
  if (!file_mapping_handle.get()) {
    return false;
  }

  handle_ = MapViewOfFile(file_mapping_handle.get(), FILE_MAP_ALL_ACCESS, 0, 0, capacity);
  return handle_ != nullptr;
}

void MmapAux::Unmap() {
  if (handle_) {
    UnmapViewOfFile(handle_);
  }
  handle_ = nullptr;
}

void MmapAux::Sync() {
  if (handle_) {
    FlushViewOfFile(handle_, capacity_);
  }
}

}  
