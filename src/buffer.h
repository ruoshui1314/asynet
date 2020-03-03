#ifndef ASYNET_BUFFER_H_
#define ASYNET_BUFFER_H_

#include <list>
#include <vector>
#include <string>

namespace asynet {

const int DEFAUTL_MAX_BUFFER_NUM = 10;
const int DEFAULT_BUFFER_SIZE = 4096;

class Buffer;

class BufferBlock {
public:
    BufferBlock();
    BufferBlock(BufferBlock&& block);
    std::size_t get_free_space() { return data_.size() - write_index_; }
    std::size_t get_used_space() { return write_index_ - read_index_; }
    std::size_t size() { return data_.size(); }
    std::string to_string();
    bool full();
    bool empty() { return read_index_ == 0 && write_index_ == 0; }
    void reset();
    void operator=(const BufferBlock& block) {
        data_ = block.data_;
        read_index_ = block.read_index_;
        write_index_ = block.write_index_;
    }
    int append_to_block(const char* p, int n);

private:
    std::vector<char> data_;
    std::size_t read_index_;
    std::size_t write_index_;
    friend Buffer;
};

using buffer_block_vec = std::vector<BufferBlock>;

class Buffer {
public:
    Buffer() = default;
    BufferBlock& get_available_block();
    int read_socket(int fd);
    int write_socket(int fd, const std::string& message);
    std::string read_all();
    void append_to_cache(const char* p, int n);
    void append_to_cache(const std::string& message);
    int write_buffer_cache(int fd);
    bool has_data();

private:
    buffer_block_vec blocks_;
    int current_index_;
};

}

#endif