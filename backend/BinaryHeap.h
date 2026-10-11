#ifndef BINARY_HEAP_H
#define BINARY_HEAP_H

#include <vector>
#include <stdexcept>
#include <utility>
#include <functional>


template <typename T, typename LowerPriority = std::less<T>>
class BinaryHeap {
private:
    std::vector<T> data;
    LowerPriority isLower;

    void siftUp(int index) {
        while (index > 0) {
            int parentIndex = (index - 1) / 2;
            // If parent has lower priority than current item swap them upwards
            if (isLower(data[parentIndex], data[index])) {
                std::swap(data[parentIndex], data[index]);
                index = parentIndex;
            } else {
                break;
            }
        }
    }

    void siftDown(int index) {
        int n = static_cast<int>(data.size());
        while (true) {
            int leftChild = 2 * index + 1;
            int rightChild = 2 * index + 2;
            int bestIndex = index;

            // left child has higher priority than current best?
            if (leftChild < n && isLower(data[bestIndex], data[leftChild])) {
                bestIndex = leftChild;
            }
            // right child has higher priority than current best?
            if (rightChild < n && isLower(data[bestIndex], data[rightChild])) {
                bestIndex = rightChild;
            }

            // If a child outranks the current nod swap and continue sifting down
            if (bestIndex != index) {
                std::swap(data[index], data[bestIndex]);
                index = bestIndex;
            } else {
                break;
            }
        }
    }

public:
    BinaryHeap() : isLower(LowerPriority()) {}
    explicit BinaryHeap(LowerPriority comp) : isLower(comp) {}

    void push(const T& value) {
        data.push_back(value);
        siftUp(static_cast<int>(data.size()) - 1);
    }

    T pop() {
        if (isEmpty()) {
            throw std::underflow_error("Heap is empty");
        }
        T rootValue = data[0];
        data[0] = data.back();
        data.pop_back();
        if (!data.empty()) {
            siftDown(0);
        }
        return rootValue;
    }

    const T& top() const {
        if (isEmpty()) {
            throw std::underflow_error("Heap is empty");
        }
        return data[0];
    }

    bool isEmpty() const {
        return data.empty();
    }

    int size() const {
        return static_cast<int>(data.size());
    }

    void clear() {
        data.clear();
    }
};

#endif // BINARY_HEAP_H
