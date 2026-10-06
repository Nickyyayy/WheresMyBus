#ifndef ROUTE_H
#define ROUTE_H

#include "RouteStopNode.h"
#include <string>
#include <iostream>

using namespace std;

// Route represents a predefined sequence of stops as a singly linked list
class Route {
private:
    string routeId;
    string routeName;
    string description;
    string startStopId;
    string restStopId;

    RouteStopNode* head;
    RouteStopNode* tail;
    int totalStops;

public:
    Route(string id, string name, string description, string startStopId, string restStopId);
    ~Route();

    Route(const Route&) = delete;
    Route& operator=(const Route&) = delete;

    // Adds a stop node to the end of the linked list
    // Returns false and does not take ownership if invalid (caller deletes node)
    bool addStopToEnd(RouteStopNode* node);

    RouteStopNode* getHead() const;
    RouteStopNode* getLastStop() const;
    RouteStopNode* findStopByIndex(int sequenceIndex) const;
    RouteStopNode* findStopById(const string& stopId) const;      // First match
    RouteStopNode* findLastStopById(const string& stopId) const;  // Last match

    int getTotalStops() const;
    string getRouteId() const;
    string getRouteName() const;
    string getDescription() const;
    string getStartStopId() const;
    string getRestStopId() const;

    void printRoute() const;
};

#endif // ROUTE_H
