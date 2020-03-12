#ifndef ASYNET_HEAP_H_
#define ASYNET_HEAP_H_

#include <vector>

namespace asynet {

template <typename T, typename compare = std::greater<T>>
class Heap {
public:
    Heap(compare comp = compare()): comp_(comp) {}
    
    bool empty() { return heap_.empty(); }
    size_t size() { return heap_.size(); }

    void push_heap(const T& t) {
        heap_.push_back(t);
        up_heap(heap_.size() - 1);
    }

    void push_heap(T&& t) {
        heap_.push_back(std::forward<T>(t));
        up_heap(heap_.size() - 1);
    }

    void pop_heap() {
        if (size() <= 1) {
            heap_.pop_back();
            return;
        }
        std::swap(heap_[0], heap_[size() - 1]);
        heap_.pop_back();
        down_heap(0);
    }

    void remove(size_t index) {
        if (index >= size())
            return;
        std::swap(heap_[index], heap_[size() - 1]);
        heap_.pop_back();
        if (index > 0 && comp_(heap_[index], heap_[(index-1)/2]))
            up_heap(index);
        else
            down_heap(index);
    }

    const T& top() {
        return heap_.front(); 
    }
private:
    void up_heap(size_t index) {
        while (index > 0) {
            size_t parent = (index - 1) / 2;
            if (comp_(heap_[parent], heap_[index]))
                break;
            std::swap(heap_[index], heap_[parent]);
            index = parent;
        }
    }

    void down_heap(size_t index) {
        size_t child = 2 * index + 1;
        while (child < heap_.size()) {
            if (child + 1 != heap_.size() &&
                comp_(heap_[child + 1], heap_[child]))
                child += 1;
            if (comp_(heap_[index], heap_[child]))
                break;
            std::swap(heap_[index], heap_[child]);
            index = child;
            child = 2 * index + 1;
        }
    }

    std::vector<T> heap_;
    compare comp_;
};
}

#endif