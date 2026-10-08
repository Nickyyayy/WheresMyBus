#include "RouteRegistry.h"
#include <unordered_set>

using namespace std;

RouteRegistry::~RouteRegistry() {
    for (auto& pair : routeMap) {
        delete pair.second;
    }
    routeMap.clear();
    routeIds.clear();
}

bool RouteRegistry::addRoute(Route* route) {
    if (route == nullptr) {
        return false;
    }
    const string& id = route->getRouteId();
    if (routeMap.find(id) != routeMap.end()) {
        return false;
    }
    routeMap[id] = route;
    routeIds.push_back(id);
    return true;
}

Route* RouteRegistry::getRouteById(const string& routeId) const {
    auto it = routeMap.find(routeId);
    if (it != routeMap.end()) {
        return it->second;
    }
    return nullptr;
}

vector<Route*> RouteRegistry::getAllRoutes() const {
    vector<Route*> result;
    result.reserve(routeIds.size());
    for (const string& id : routeIds) {
        auto it = routeMap.find(id);
        if (it != routeMap.end()) {
            result.push_back(it->second);
        }
    }
    return result;
}

int RouteRegistry::getRouteCount() const {
    return static_cast<int>(routeMap.size());
}

vector<pair<string, string>> RouteRegistry::getDistinctStops() const {
    vector<pair<string, string>> distinctStops;
    unordered_set<string> seenStopIds;

    for (const string& id : routeIds) {
        auto it = routeMap.find(id);
        if (it == routeMap.end() || it->second == nullptr) {
            continue;
        }
        RouteStopNode* current = it->second->getHead();
        while (current != nullptr) {
            if (seenStopIds.insert(current->getStopId()).second) {
                distinctStops.emplace_back(current->getStopId(), current->getStopName());
            }
            current = current->getNext();
        }
    }
    return distinctStops;
}
