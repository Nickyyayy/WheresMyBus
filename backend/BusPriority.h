#ifndef BUS_PRIORITY_H
#define BUS_PRIORITY_H

#include "Bus.h"
#include "BinaryHeap.h"
#include <string>
#include <vector>

using namespace std;

inline bool busIdComesBefore(const string& a, const string& b) {
    if (a.length() != b.length()) {
        return a.length() < b.length();
    }
    return a < b;
}

inline bool isActiveForDisplay(const Bus* b) {
    if (b == nullptr) {
        return false;
    }
    return b->getState() == BusState::IN_SERVICE || b->getState() == BusState::RESTING;
}


// priority function for queue
struct BusDisplayPriority {
    bool operator()(const Bus* a, const Bus* b) const {
        if (a == nullptr && b == nullptr) return false;
        if (a == nullptr) return true;
        if (b == nullptr) return false;

        bool aActive = isActiveForDisplay(a);
        bool bActive = isActiveForDisplay(b);

        //  active buses above inactive buses
        if (aActive != bActive) {
            return !aActive && bActive;
        }

        // Furthest along the route above those at earlier stops
        if (a->getCurrentSequenceIndex() != b->getCurrentSequenceIndex()) {
            return a->getCurrentSequenceIndex() < b->getCurrentSequenceIndex();
        }

        // smaller bus number has higher priority
        return busIdComesBefore(b->getBusId(), a->getBusId());
    }
};

// Sorts a list of buses according to display priority 
inline vector<Bus*> sortBusesForDisplay(const vector<Bus*>& buses) {
    BinaryHeap<Bus*, BusDisplayPriority> heap;
    for (Bus* b : buses) {
        heap.push(b);
    }
    vector<Bus*> result;
    result.reserve(buses.size());
    while (!heap.isEmpty()) {
        result.push_back(heap.pop());
    }
    return result;
}

#endif 
