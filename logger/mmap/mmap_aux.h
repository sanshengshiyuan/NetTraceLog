#pragma once

#include <filesystem>
#include <memory>

#include "../utils/file_util.h"
#include "../utils/sys_util.h"

namespace logger {

class MmapAux {
public:
    static constexpr size_t kDefaultCapacity = 512 * 1024;

    static size_t GetValidCapacity(size_t size) {
        size_t page_size = GetPageSize();
        return (size + page_size - 1)/ page_size * page_size;
    }
    using fpath = std::filesystem::path;//使用别名简化类型

    explicit MmapAux(fpath file_path);//单参数构造函数采用explicit避免隐式转换

    ~MmapAux() = default;

    MmapAux(const MmapAux&) = delete;//禁用拷贝构造

    MmapAux& operator = (const MmapAux&) = delete;//禁用赋值构造

    void Resize(size_t new_size);//重新分配内存大小

    void EnsureCapacity(size_t new_size);

    size_t Size() const;//返回内存大小


    uint8_t* Data() const;//这里返回的是数据的指针，我这里是抄的源代码，暂时不知道为什么返回的是一个uint8_t*,待会在看一下

    void Clear(); //类似vector清除所有数据

    void Push(const void* data, size_t size);// 这里是将数据送到mmap缓存中，要指定大小

    double GetRatio() const;//获得数据和空间的比例，当超过一定比例就触发写操作或者交换缓存
    //看缓存是否为空
    bool Empty() const { 
        return Size() == 0;
    }

private:
    struct MmapHeader {
        static constexpr uint32_t kmagic = 0xdadad;
        uint32_t magic = kmagic;//魔数标识这是个mmap缓存
        uint32_t size;//标识后面的数据大小
    };

private:
    void Reserve(size_t new_size); //重新分配mmap缓存空间，传入新的空间大小

    size_t Capacity() const noexcept;

    bool TryMap(size_t capacity); //尝试为文件映射指定大小的地址空间；

    void Unmap(); //解除文件和地址空间的映射

    bool Isvalid() const; //验证这块mmap是否有效，主要看魔数是否正确；

    MmapHeader* Header() const; //返回这块mmap的头部指针；

    void Init(); //初始化这块mmap;

    void Sync();

private:
    fpath file_path_;//mmap映射的文件路径
    void* handle_;
    size_t capacity_; //mmap缓存容量
};
}