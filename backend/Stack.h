#ifndef STACK_H
#define STACK_H

#include <stdexcept>
#include <vector>

// Hand-written templated Stack implemented as a linked list
template <typename T>
class Stack {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& value, Node* nextNode) : data(value), next(nextNode) {}
    };

    Node* topNode;
    int count;

public:
    Stack() : topNode(nullptr), count(0) {}

    ~Stack() {
        clear();
    }

    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    void push(const T& value) {
        topNode = new Node(value, topNode);
        count++;
    }

    T pop() {
        if (isEmpty()) {
            throw std::underflow_error("Stack is empty");
        }
        Node* oldTop = topNode;
        T result = oldTop->data;
        topNode = topNode->next;
        delete oldTop;
        count--;
        return result;
    }

    T top() const {
        if (isEmpty()) {
            throw std::underflow_error("Stack is empty");
        }
        return topNode->data;
    }

    bool isEmpty() const {
        return topNode == nullptr;
    }

    int size() const {
        return count;
    }

    void clear() {
        while (topNode != nullptr) {
            Node* nextNode = topNode->next;
            delete topNode;
            topNode = nextNode;
        }
        count = 0;
    }

    // Returns a copy of the elements from top to bottom (newest first)
    // Allows reporting and inspection without modifying the stack
    std::vector<T> toVector() const {
        std::vector<T> result;
        result.reserve(count);
        Node* curr = topNode;
        while (curr != nullptr) {
            result.push_back(curr->data);
            curr = curr->next;
        }
        return result;
    }
};

#endif // STACK_H
