#ifndef ROUTE_STOP_NODE_H
#define ROUTE_STOP_NODE_H

#include "Stop.h"
#include <string>

using namespace std;

// RouteStopNode represents a single stop entry along an ordered bus route linked list
class RouteStopNode {
public:
    static const int NO_ETA = -1;                 // Last node: nothing comes next
    static constexpr double NO_DISTANCE = -1.0;   // Database NULL distance

private:
    string stopId;
    string stopName;
    StopType stopType;
    bool isRestStop;
    int sequenceIndex;             // 1-based position in route
    int etaToNextMinutes;          // ETA to next stop in minutes
    double distanceToNextKm;       // Distance to next stop in km
    bool etaIsProvisional;         // Flag if ETA is provisional
    RouteStopNode* next;           // Pointer to next stop node in route

public:
    RouteStopNode(const Stop& stop, int sequenceIndex, int etaToNextMinutes,
                  double distanceToNextKm, bool etaIsProvisional);

    string getStopId() const;
    string getStopName() const;
    StopType getStopType() const;
    bool getIsRestStop() const;
    int getSequenceIndex() const;

    bool hasEtaToNext() const;
    int getEtaToNextMinutes() const;
    bool hasDistanceToNext() const;
    double getDistanceToNextKm() const;
    bool getEtaIsProvisional() const;

    RouteStopNode* getNext() const;
    void setNext(RouteStopNode* nextNode);
};

#endif // ROUTE_STOP_NODE_H
