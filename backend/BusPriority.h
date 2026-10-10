#ifndef BUS_PRIORITY_H
#define BUS_PRIORITY_H

#include "Bus.h"
#include "BinaryHeap.h"
#include <string>
#include <vector>

using namespace std;

// Compares two bus IDs by numeric length first, then lexicographically
// e.g. "9" comes before "21" which comes before "118"
inline bool busIdComesBefore(const string& a, const string& b) {
    if (a.length() != b.length()) {
        return a.length() < b.length();
    }
    return a < b;
}

// Determines if a bus is considered actively running on route
inline bool isActiveForDisplay(const Bus* b) {
    if (b == nullptr) {
        return false;
    }
    return b->getState() == BusState::IN_SERVICE || b->getState() == BusState::RESTING;
}

// Comparator defining display priority for the BinaryHeap
// Returns true when 'a' has LOWER priority than 'b'
struct BusDisplayPriority {
    bool operator()(const Bus* a, const Bus* b) const {
        if (a == nullptr && b == nullptr) return false;
        if (a == nullptr) return true;
        if (b == nullptr) return false;

        bool aActive = isActiveForDisplay(a);
        bool bActive = isActiveForDisplay(b);

        // 1. Active buses outrank inactive buses
        if (aActive != bActive) {
            return !aActive && bActive;
        }

        // 2. Furthest along the route outranks earlier stops
        if (a->getCurrentSequenceIndex() != b->getCurrentSequenceIndex()) {
            return a->getCurrentSequenceIndex() < b->getCurrentSequenceIndex();
        }

        // 3. Tie-break: smaller bus number has higher priority
        // 'a' has lower priority if 'b' comes before 'a'
        return busIdComesBefore(b->getBusId(), a->getBusId());
    }
};

// Sorts a list of buses according to display priority using the custom BinaryHeap
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

#endif // BUS_PRIORITY_H
