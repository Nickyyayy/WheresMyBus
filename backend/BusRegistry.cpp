#include "BusRegistry.h"
#include <algorithm>

using namespace std;

BusRegistry::~BusRegistry() {
    for (auto& pair : busMap) {
        delete pair.second;
    }
    busMap.clear();
    busIds.clear();
}

bool BusRegistry::addBus(Bus* bus) {
    if (bus == nullptr) {
        return false;
    }
    const string& id = bus->getBusId();
    if (busMap.find(id) != busMap.end()) {
        return false; // Duplicate
    }
    busMap[id] = bus;
    busIds.push_back(id);
    return true;
}

Bus* BusRegistry::getBusById(const string& busId) const {
    auto it = busMap.find(busId);
    if (it != busMap.end()) {
        return it->second;
    }
    return nullptr;
}

bool BusRegistry::removeBus(const string& busId) {
    auto it = busMap.find(busId);
    if (it == busMap.end()) {
        return false;
    }
    delete it->second;
    busMap.erase(it);

    auto idIt = find(busIds.begin(), busIds.end(), busId);
    if (idIt != busIds.end()) {
        busIds.erase(idIt);
    }
    return true;
}

vector<Bus*> BusRegistry::getAllBuses() const {
    vector<Bus*> result;
    result.reserve(busIds.size());
    for (const string& id : busIds) {
        auto it = busMap.find(id);
        if (it != busMap.end()) {
            result.push_back(it->second);
        }
    }
    return result;
}

vector<Bus*> BusRegistry::getBusesForRoute(const string& routeId) const {
    vector<Bus*> result;
    for (const string& id : busIds) {
        auto it = busMap.find(id);
        if (it != busMap.end() && it->second->getRouteId() == routeId) {
            result.push_back(it->second);
        }
    }
    return result;
}

int BusRegistry::getBusCount() const {
    return static_cast<int>(busMap.size());
}
