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

int BufferBlock::append_to_block(const char* p, int n) {
    int length = n <= get_free_space() ? n : get_free_space();
    data_.insert(data_.begin() + write_index_, p, p+length);
    write_index_ += length;
    return length;
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

BufferBlock& Buffer::get_available_block() {
    if (current_index_ != -1 && current_index_ < blocks_.size()) {
        if (blocks_[current_index_].get_free_space() != 0)
            return blocks_[current_index_];
        if (current_index_ + 1 < blocks_.size())
            return blocks_[current_index_];
    }
    blocks_.push_back(BufferBlock());
    ++current_index_;
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
        n = ::write(fd, &block.data_[block.read_index_], block.get_used_space());
        if (n <= 0)
            break;
         block.read_index_ += n;
        if (block.full()) {
            block.reset();
            blocks.push_back(std::move(block));
            ++iter;
        }
    }
    if (iter != blocks_.begin()) {
        current_index_ -= std::distance(blocks_.begin(), iter);
        blocks_.erase(blocks_.begin(), iter);
    }
    for (auto& block : blocks) {
        if (blocks_.size() > DEFAUTL_MAX_BUFFER_NUM)
            break;
        blocks_.push_back(std::move(block));
    }
    return n;
}

void Buffer::append_to_cache(const char* p, int length) {
    int n;
    int write = 0;
    while (length > 0) {
        BufferBlock& block = get_available_block();
        n = block.append_to_block(p + write, length);
        length -= n;
        write += n;
    }
}

bool Buffer::has_data() {
    if (blocks_.empty())
        return false;
    return blocks_[current_index_].get_used_space() != 0;
}

int Buffer::write_socket(int fd, const std::string& message) {
    const char* data = message.c_str();
    int index = 0;
    int length = message.size();
    int n;
    if (has_data()) {
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
    current_index_ = -1;
    return ss.str();
}
