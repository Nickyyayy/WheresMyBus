#ifndef ROUTE_REGISTRY_H
#define ROUTE_REGISTRY_H

#include "Route.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

using namespace std;


class RouteRegistry {
private:
    unordered_map<string, Route*> routeMap;
    vector<string> routeIds; // Deterministic insertion order

public:
    RouteRegistry() = default;
    ~RouteRegistry();

    RouteRegistry(const RouteRegistry&) = delete;
    RouteRegistry& operator=(const RouteRegistry&) = delete;

    bool addRoute(Route* route);

    Route* getRouteById(const string& routeId) const;
    vector<Route*> getAllRoutes() const;
    int getRouteCount() const;

    // Returns every distinct stop used by any route as (stopId, stopName) in order of first appearance
    vector<pair<string, string>> getDistinctStops() const;
};

#endif
