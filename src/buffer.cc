#include "buffer.h"

#include <sstream>
#include <unistd.h>
#include <sys/uio.h>
#include <cstring>

using namespace asynet;

BufferBlock::BufferBlock(int size) {
    data_.resize(size);
    index_ = 0;
    offset_ = 0;
    next_ = nullptr;
}

int BufferBlock::append(const char* p, size_t n) {
    size_t size = n <= length() ? n : length();
    ::memcpy(&data_[offset_], p, size);
    offset_ += size;
    return size;
}

void BufferBlock::reset() {
    index_ = 0;
    offset_ = 0;
}

std::string BufferBlock::to_string() {
    std::string s(&data_[index_], &data_[offset_]);
    index_ = offset_;
    return s; 
}

BufferBlock* ReadBuffer::get_readable_block() {
    if (read_head_ == nullptr) {
        read_head_ = new BufferBlock();
        read_tail_ = read_head_;
        return read_tail_;
    }
    if (read_tail_->length() != 0)
        return read_tail_;
    BufferBlock* block = new BufferBlock();
    read_tail_->next_ = block;
    read_tail_ = block;
    return read_tail_;
}

int ReadBuffer::read_fd(int fd) {
    struct iovec vec[2];
    char buf[65536];
    BufferBlock* block = get_readable_block();
    vec[0].iov_len = block->length();
    vec[0].iov_base = &block->data_[block->offset_];
    vec[1].iov_len = sizeof(buf);
    vec[1].iov_base = buf;
    ssize_t n = ::readv(fd, vec, 2);
    if (n < 0)
        return n;
    if ((size_t)n < block->length()) {
        block->offset_ += n;
    } else {
        int read_size = block->length();
        block->offset_ = block->capacity();
        append(buf, (size_t)(n - read_size));
    }
    return n;
}

void ReadBuffer::append(const char* p, int length) {
    int n;
    int write = 0;
    while (length > 0) {
        BufferBlock* block = get_readable_block();
        n = block->append(p + write, length);
        length -= n;
        write += n;
    }
}

std::string ReadBuffer::read_all() {
    if (read_head_ == nullptr)
        return std::string();
    std::stringstream ss;
    BufferBlock* cur = read_head_;
    BufferBlock* next;
    while (cur != nullptr) {
        ss.write(&cur->data_[cur->index_], cur->size());
        next = cur->next_;
        delete cur;
        cur = next;
    }
    read_head_ = nullptr;
    read_tail_ = nullptr;
    return ss.str();
}

void ReadBuffer::clear() {
    BufferBlock* cur = read_head_;
    BufferBlock* next;
    while (cur) {
        next = cur->next_;
        delete cur;
        cur = next;
    }
    read_head_ = nullptr;
    read_tail_ = nullptr;
}

int WriteBuffer::write_fd(int fd, const std::string& msg) {
    BufferBlock* req = new BufferBlock(msg.size());
    req->append(msg.c_str(), msg.size());
    BufferBlock* prev_head = head_.exchange(req, std::memory_order_release);
    // someone is writing.
    if (prev_head != nullptr) {
        req->next_ = prev_head;
        return 0;
    }
    int n = write_fd(fd, req);
    if (n <= 0)
        return n;
    req->index_ += n;

    complete_ = is_write_complete(req, true);
    if (complete_)
        delete req;
    else
        write_head_ = req;
    return n;
}

int WriteBuffer::write_fd(int fd, BufferBlock* head) {
    struct iovec vec[MAX_IOVEC_NUM];
    size_t nvec;
    BufferBlock* cur = head;
    for (nvec = 0; nvec < MAX_IOVEC_NUM && cur; ++nvec) {
        vec[nvec].iov_base = &head->data_[head->index_];
        vec[nvec].iov_len = head->size();
        cur = cur->next_;
    }

    ssize_t n = ::writev(fd, vec, nvec);
    return n;
}

int WriteBuffer::keep_write(int fd) {
    while (1) {
        if (write_head_->next_ != nullptr && write_head_->empty()) {
            BufferBlock* save = write_head_;
            write_head_ = write_head_->next_;
            delete save;
        }
        int length = write_fd(fd, write_head_);
        if (length <= 0)
            return length;
        // Release WriteRequest until non-empty data or last request.
        int total = length;
        BufferBlock* cur;
        while (write_head_ != nullptr && total > 0) {
            cur = write_head_;
            size_t size = std::min(cur->size(), (size_t)length);
            cur->index_ += size;
            total -= size;
            if (cur->empty() && cur->next_) {
                write_head_ = cur->next_;
                delete cur;
            }
        }
        // Return when there's no more WriteRequests and req is completely
        // written.
        complete_ = is_write_complete(write_tail_, (write_head_ == write_tail_));
        if (complete_) {
            delete write_head_;
            write_head_ = nullptr;
            write_tail_ = nullptr;
            return length;
        }
        return length;
    }
}

bool WriteBuffer::is_write_complete(BufferBlock* old_head, bool single) {
    bool no_data = true;
    BufferBlock* desired = nullptr;
    if (!old_head->empty() || !single) {
        desired = old_head;
        // Write is obviously not complete if old_head is not fully written.
        complete_ = false;
    }
    // Remove block when data has been sended.
    BufferBlock* new_head = old_head;
    if (head_.compare_exchange_strong(new_head, desired, std::memory_order_acquire))
        return no_data;

    // Added new requests.
    BufferBlock* tail = nullptr;
    BufferBlock* p = new_head;
    do {
        BufferBlock* next = p->next_;
        p->next_ = tail;
        tail = p;
        p = next;
    } while (p != old_head);

    // Link old list with new list.
    old_head->next_ = new_head;
    write_tail_ = new_head;
    return false;
}

void WriteBuffer::clear() {
    complete_ = true;
    BufferBlock* head = head_.exchange(nullptr, std::memory_order_relaxed);
    if (head) {
        BufferBlock* tail = nullptr;
        BufferBlock* p = head;
        do {
            BufferBlock* next = p->next_;
            p->next_ = tail;
            tail = p;
            p = next;
        } while (p != write_tail_);
        if (write_tail_)
            write_tail_->next_ = head;
    }
    BufferBlock* cur = write_head_;
    BufferBlock* next;
    while (cur) {
        next = cur->next_;
        delete cur;
        cur = next;
    }
    write_head_ = nullptr;
    write_tail_ = nullptr;
}