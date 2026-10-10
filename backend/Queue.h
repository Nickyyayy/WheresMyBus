#ifndef QUEUE_H
#define QUEUE_H

#include <stdexcept>

// Hand-written templated Queue implemented as a singly linked list
template <typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& value) : data(value), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    int count;

public:
    Queue() : head(nullptr), tail(nullptr), count(0) {}

    ~Queue() {
        clear();
    }

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    void push(const T& value) {
        Node* newNode = new Node(value);
        if (tail == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        count++;
    }

    T pop() {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty");
        }
        Node* oldHead = head;
        T result = oldHead->data;
        head = head->next;
        if (head == nullptr) {
            tail = nullptr;
        }
        delete oldHead;
        count--;
        return result;
    }

    T front() const {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty");
        }
        return head->data;
    }

    bool isEmpty() const {
        return head == nullptr;
    }

    int size() const {
        return count;
    }

    void clear() {
        while (head != nullptr) {
            Node* nextNode = head->next;
            delete head;
            head = nextNode;
        }
        tail = nullptr;
        count = 0;
    }
};

#endif // QUEUE_H
