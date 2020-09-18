#ifndef ASYNET_BUFFER_H_
#define ASYNET_BUFFER_H_

#include <vector>
#include <string>
#include <atomic>
#include <list>

namespace asynet {

const int DEFAULT_BUFFER_SIZE = 4096;

class BufferBlock {
public:
    BufferBlock(int size = DEFAULT_BUFFER_SIZE);

    BufferBlock(BufferBlock&& block);

    std::size_t length() { return data_.size() - offset_; }

    std::size_t size() { return offset_ - index_; }

    std::size_t capacity() { return data_.size(); }

    std::string to_string();

    bool empty() { return offset_ == index_; }

    void reset();

    int append(const char* p, size_t n);

private:
    friend class ReadBuffer;
    friend class WriteBuffer;
    std::vector<char> data_;
    std::size_t index_;
    std::size_t offset_;
    BufferBlock* next_;
};

class ReadBuffer {
public:
    ReadBuffer(): read_head_(nullptr), read_tail_(nullptr) {}

    int read_fd(int fd);

    std::string read_all();

    void append(const char* p, int n);

    void clear();

private:
    BufferBlock* get_readable_block();

private:
    BufferBlock* read_head_;
    BufferBlock* read_tail_;
};

class WriteBuffer {
public:
    WriteBuffer(): head_(nullptr), write_head_(nullptr), write_tail_(nullptr), complete_(true) {}

    int write_fd(int fd, const std::string& msg);

    bool is_complete() { return complete_; }

    int keep_write(int fd);

    void clear();

private:
    int write_fd(int fd, BufferBlock* head);

    bool is_write_complete(BufferBlock* old_head, bool single);

private:
    const size_t MAX_IOVEC_NUM = 16;
    std::atomic<BufferBlock*> head_;
    BufferBlock* write_head_;
    BufferBlock* write_tail_;
    bool complete_;
};

}

#endif