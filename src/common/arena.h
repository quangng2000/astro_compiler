#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <vector>

class Arena {
public:
    Arena() { add_block(); }
    ~Arena() = default;

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    Arena(Arena&&) = default;
    Arena& operator=(Arena&&) = default;

    template<typename T, typename... Args>
    T* alloc(Args&&... args) {
        void* mem = alloc_raw(sizeof(T), alignof(T));
        return new (mem) T{std::forward<Args>(args)...};
    }

    size_t block_count() const { return blocks_.size(); }

private:
    static constexpr size_t BLOCK_SIZE = 4096;

    struct Block {
        std::unique_ptr<char[]> data;
        size_t used = 0;
    };

    std::vector<Block> blocks_;

    void add_block() {
        blocks_.push_back({std::make_unique<char[]>(BLOCK_SIZE), 0});
    }

    void* alloc_raw(size_t size, size_t align) {
        auto& block = blocks_.back();
        size_t padding = (align - (block.used % align)) % align;
        size_t needed = padding + size;

        if (block.used + needed > BLOCK_SIZE) {
            size_t new_size = size > BLOCK_SIZE ? size : BLOCK_SIZE;
            blocks_.push_back({std::make_unique<char[]>(new_size), 0});
            return alloc_raw(size, align);
        }

        block.used += padding;
        void* ptr = block.data.get() + block.used;
        block.used += size;
        return ptr;
    }
};
