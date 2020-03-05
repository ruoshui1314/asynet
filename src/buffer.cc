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
    if (!used_blocks_.empty()) {
        BufferBlock& block = used_blocks_.back();
        if (block.get_free_space() != 0)
            return block;
    }
    if (!free_blocks_.empty()) {
        used_blocks_.push_back(std::move(free_blocks_.front()));
        free_blocks_.pop_front();
    } else {
        used_blocks_.push_back(BufferBlock());
    }
    return used_blocks_.back();
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

int Buffer::send_buffer_cache(int fd) {
    int n;
    while (!used_blocks_.empty()) {
        BufferBlock& block = used_blocks_.front();
        n = ::write(fd, &block.data_[block.read_index_], block.get_used_space());
        if (n <= 0)
            break;
         block.read_index_ += n;
        if (!block.full())
            continue;
        add_to_free_list(std::move(block));
        used_blocks_.pop_front();
    }
    return n;
}

void Buffer::append_to_buffer(const char* p, int length) {
    int n;
    int write = 0;
    while (length > 0) {
        BufferBlock& block = get_available_block();
        n = block.append_to_block(p + write, length);
        length -= n;
        write += n;
    }
}

int Buffer::write_socket(int fd, const std::string& message) {
    const char* data = message.c_str();
    int index = 0;
    int length = message.size();
    int n;
    if (!used_blocks_.empty()) {
        n = send_buffer_cache(fd);
        if (n <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                append_to_buffer(data, length);
            return n;
        }
    }
    while (length > 0) {
        n = ::write(fd, data, length);
        if (n <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                append_to_buffer(data, length);
            return n;
        }
        data += n;
        length -= n;
    }
    return n;
}

std::string Buffer::read_all() {
    if (used_blocks_.empty())
        return std::string();
    std::stringstream ss;
    for (auto& block : used_blocks_) {
        ss << block.to_string();
        add_to_free_list(std::move(block));
    }
    used_blocks_.clear();
    return ss.str();
}

void Buffer::add_to_free_list(BufferBlock&& block) {
    if (free_blocks_.size() > DEFAUTL_MAX_BUFFER_NUM)
        return;
    block.reset();
    free_blocks_.push_back(std::forward<BufferBlock>(block));
}
