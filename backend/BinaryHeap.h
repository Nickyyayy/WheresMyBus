#ifndef BINARY_HEAP_H
#define BINARY_HEAP_H

#include <queue>
#include <vector>
#include <functional>
#include <stdexcept>

// STL-backed Binary Heap adapter using std::priority_queue
template <typename T, typename LowerPriority = std::less<T>>
class BinaryHeap {
private:
    LowerPriority isLower;
    std::priority_queue<T, std::vector<T>, LowerPriority> pq;

public:
    BinaryHeap() : isLower(LowerPriority()), pq(LowerPriority()) {}
    explicit BinaryHeap(LowerPriority comp) : isLower(comp), pq(comp) {}

    void push(const T& value) {
        pq.push(value);
    }

    T pop() {
        if (isEmpty()) {
            throw std::underflow_error("Heap is empty");
        }
        T rootValue = pq.top();
        pq.pop();
        return rootValue;
    }

    const T& top() const {
        if (isEmpty()) {
            throw std::underflow_error("Heap is empty");
        }
        return pq.top();
    }

    bool isEmpty() const {
        return pq.empty();
    }

    bool empty() const {
        return pq.empty();
    }

    int size() const {
        return static_cast<int>(pq.size());
    }

    void clear() {
        pq = std::priority_queue<T, std::vector<T>, LowerPriority>(isLower);
    }
};

#endif // BINARY_HEAP_H
