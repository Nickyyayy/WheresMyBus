#ifndef QUEUE_H
#define QUEUE_H

#include <queue>
#include <stdexcept>

// STL-backed Queue adapter using std::queue
template <typename T>
class Queue {
private:
    std::queue<T> impl;

public:
    Queue() = default;

    void push(const T& value) {
        impl.push(value);
    }

    T pop() {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty");
        }
        T result = impl.front();
        impl.pop();
        return result;
    }

    T front() const {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty");
        }
        return impl.front();
    }

    bool isEmpty() const {
        return impl.empty();
    }

    bool empty() const {
        return impl.empty();
    }

    int size() const {
        return static_cast<int>(impl.size());
    }

    void clear() {
        std::queue<T> emptyQueue;
        std::swap(impl, emptyQueue);
    }
};

#endif // QUEUE_H
