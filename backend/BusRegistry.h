#ifndef BUS_REGISTRY_H
#define BUS_REGISTRY_H

#include "Bus.h"
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

// BusRegistry owns Bus objects in memory using a hash table
// Maintains insertion order for deterministic display
class BusRegistry {
private:
    unordered_map<string, Bus*> busMap;
    vector<string> busIds; // Deterministic insertion order

public:
    BusRegistry() = default;
    ~BusRegistry();

    BusRegistry(const BusRegistry&) = delete;
    BusRegistry& operator=(const BusRegistry&) = delete;

    // Takes ownership of bus on success. Returns false if nullptr or duplicate ID (caller keeps ownership).
    bool addBus(Bus* bus);

    Bus* getBusById(const string& busId) const;
    bool removeBus(const string& busId);

    vector<Bus*> getAllBuses() const;
    vector<Bus*> getBusesForRoute(const string& routeId) const;
    int getBusCount() const;
};

#endif // BUS_REGISTRY_H
