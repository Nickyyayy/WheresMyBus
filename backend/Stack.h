#ifndef STACK_H
#define STACK_H

#include <stack>
#include <vector>
#include <stdexcept>

// STL-backed Stack adapter using std::stack
template <typename T>
class Stack {
private:
    std::stack<T> impl;

public:
    Stack() = default;

    void push(const T& value) {
        impl.push(value);
    }

    T pop() {
        if (isEmpty()) {
            throw std::underflow_error("Stack is empty");
        }
        T result = impl.top();
        impl.pop();
        return result;
    }

    T top() const {
        if (isEmpty()) {
            throw std::underflow_error("Stack is empty");
        }
        return impl.top();
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
        while (!impl.empty()) {
            impl.pop();
        }
    }

    // Returns a copy of the elements from top to bottom (newest first)
    std::vector<T> toVector() const {
        std::vector<T> result;
        result.reserve(impl.size());
        std::stack<T> copy = impl;
        while (!copy.empty()) {
            result.push_back(copy.top());
            copy.pop();
        }
        return result;
    }
};

#endif // STACK_H
