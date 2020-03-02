#include "buffer.h"

#include <sstream>
#include <unistd.h>

using namespace asynet;

BufferBlock::BufferBlock() {
    data_.resize(DEFAULT_BUFFER_SIZE);
    read_index_ = 0;
    write_index_ = 0;
}

BufferBlock::BufferBlock(BufferBlock&& block) {
    std::swap(block.data_, data_);
    std::swap(block.read_index_, read_index_);
    std::swap(block.write_index_, write_index_);
}

void BufferBlock::reset() {
    read_index_ = 0;
    write_index_ = 0;
}

bool BufferBlock::full() {
    return read_index_ == DEFAULT_BUFFER_SIZE;
}

std::string BufferBlock::to_string() {
    std::string s(&data_[read_index_], &data_[write_index_]);
    read_index_ = write_index_;
    return s; 
}

Buffer::Buffer(): current_index_(0) {}

BufferBlock& Buffer::get_available_block() {
    if (get_free_space() == 0)
        blocks_.push_back(BufferBlock());
    return blocks_.back();
}

int Buffer::read_socket(int fd) {
    BufferBlock& block = get_available_block();
    char* current = &block.data_[block.write_index_];
    int length = block.get_free_space();
    int n = ::read(fd, current, block.get_free_space());
    if (n <= 0)
        return n;
    block.write_index_ += n;
    return n;
}

int Buffer::write_buffer_cache(int fd) {
    int n;
    auto iter = blocks_.begin();
    buffer_block_vec blocks;
    while (iter != blocks_.end()) {
        BufferBlock& block = *iter;
        if (block.empty())
            break;
        n = ::write(fd, &block.data_[block.read_index_], block.get_free_space());
        if (n <= 0)
            break;
         block.read_index_ += n;
        if (block.full()) {
            block.reset();
            blocks.push_back(std::move(block));
            ++iter;
        }
    }
    if (iter != blocks_.begin())
        blocks_.erase(blocks.begin(), iter);
    for (auto& block : blocks)
        blocks_.push_back(std::move(block));
    return n;
}

void Buffer::append_to_cache(const char* p, int n) {
    int write;
    while (n > 0) {
        write = 0;
    }
}

int Buffer::write_socket(int fd, std::string&& message) {
    const char* data = message.c_str();
    int index = 0;
    int length = message.size();
    int n;
    if (!blocks_.empty()) {
        n = write_buffer_cache(fd);
        if (n <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                append_to_cache(data, length);
            return n;
        }
    }
    while (length > 0) {
        n = ::write(fd, data, length);
        if (n <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                append_to_cache(data, length);
            return n;
        }
        data += n;
        length -= n;
    }
    return n;
}

std::size_t Buffer::get_total_space() {
    return blocks_.size() * DEFAULT_BUFFER_SIZE;
}

std::size_t Buffer::get_free_space() {
    if (blocks_.empty())
        return 0;
    return (blocks_.size() - current_index_) * DEFAULT_BUFFER_SIZE +
        blocks_[current_index_].get_free_space();
}

std::string Buffer::read_all() {
    if (blocks_.empty())
        return std::string();
    BufferBlock& block = blocks_[current_index_];
    if (block.read_index_ == block.write_index_)
        return std::string();

    std::stringstream ss;
    for (auto& block : blocks_) {
        ss << block.to_string();
        block.reset();
    }
    current_index_ = 0;
    return ss.str();
}
