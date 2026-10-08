#ifndef BINARY_HEAP_H
#define BINARY_HEAP_H

#include <vector>
#include <stdexcept>
#include <utility>
#include <functional>

/*
 * ============================================================================
 * EXPLANATORY NOTE: THE UPSIDE-DOWN FAMILY TREE ANALOGY
 * ============================================================================
 * Think of a Binary Heap as an upside-down family tree where:
 * - The supreme ancestor (the root) sits at the very top (index 0).
 * - Every parent has up to two children directly below them:
 *     Left child is at index:  2 * i + 1
 *     Right child is at index: 2 * i + 2
 * - And for any child at index i, their direct parent is at: (i - 1) / 2
 *
 * In our priority heap, the most deserving individual (highest priority) must
 * always sit at the very top (the root).
 *
 * Sift-Up ("Promotion"):
 * When a newcomer joins the family (push), they start at the bottom of the tree
 * (the end of the array). We then compare the newcomer with their parent.
 * If the newcomer has higher priority than their parent, they trade places!
 * The newcomer keeps "climbing the ladder" until they reach a parent with
 * equal or higher standing, or until they become the root of the entire family.
 *
 * Sift-Down ("Succession"):
 * When the top leader departs (pop), the family temporarily promotes the
 * youngest member (the last element) to the top position to keep the tree
 * complete. But that member may not deserve the throne! We look at both of
 * their children, pick whichever child has the highest priority, and if that
 * child outranks the current leader, they swap places. The member keeps
 * stepping down until both children have lower priority, or until they reach
 * the bottom leaves of the tree.
 * ============================================================================
 */

template <typename T, typename LowerPriority = std::less<T>>
class BinaryHeap {
private:
    std::vector<T> data;
    LowerPriority isLower;

    void siftUp(int index) {
        while (index > 0) {
            int parentIndex = (index - 1) / 2;
            // If parent has lower priority than current item, swap them upwards
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

            // Check if left child has higher priority than current best
            if (leftChild < n && isLower(data[bestIndex], data[leftChild])) {
                bestIndex = leftChild;
            }
            // Check if right child has higher priority than current best
            if (rightChild < n && isLower(data[bestIndex], data[rightChild])) {
                bestIndex = rightChild;
            }

            // If a child outranks the current node, swap and continue sifting down
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
