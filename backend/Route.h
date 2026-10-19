#ifndef ROUTE_H
#define ROUTE_H

#include "RouteStopNode.h"
#include <string>
#include <iostream>

using namespace std;

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

   
    bool addStopToEnd(RouteStopNode* node);

    RouteStopNode* getHead() const;
    RouteStopNode* getLastStop() const;
    RouteStopNode* findStopByIndex(int sequenceIndex) const;
    RouteStopNode* findStopById(const string& stopId) const;      
    RouteStopNode* findLastStopById(const string& stopId) const;  

    int getTotalStops() const;
    string getRouteId() const;
    string getRouteName() const;
    string getDescription() const;
    string getStartStopId() const;
    string getRestStopId() const;

    void printRoute() const;
};

#endif 
