#include "mlog/async_queue.h"
#include <cstring>
#include <new>
#include <iostream>

namespace mlog {

AsyncQueue::AsyncQueue(size_t capacity)
    : size_(0), capacity_(capacity) {
    head_ = new LogMessageNode;
    head_->next = nullptr;
    head_->msg = nullptr;
    tail_.store(head_, std::memory_order_relaxed);
}

AsyncQueue::~AsyncQueue() {
    while (head_ != tail_.load(std::memory_order_acquire)) {
        LogMessageNode* node = head_;
        head_ = head_->next;
        if (node->msg) {
            delete node->msg;
        }
        delete node;
    }
    delete head_;
}

bool AsyncQueue::push(LogMessageNode* node) {
    if (!node) return false;

    node->next = nullptr;

    LogMessageNode* prev_tail = tail_.exchange(node, std::memory_order_acq_rel);
    prev_tail->next = node;

    size_.fetch_add(1, std::memory_order_acq_rel);
    return true;
}

LogMessageNode* AsyncQueue::pop() {
    LogMessageNode* old_head = head_;
    LogMessageNode* next = old_head->next;

    if (next == nullptr) {
        return nullptr;
    }

    head_ = next;
    size_.fetch_sub(1, std::memory_order_acq_rel);

    return old_head;
}

bool AsyncQueue::empty() const {
    return size_.load(std::memory_order_acquire) == 0;
}

size_t AsyncQueue::size() const {
    return size_.load(std::memory_order_acquire);
}

void AsyncQueue::set_capacity(size_t capacity) {
    capacity_ = capacity;
}

} // namespace mlog
