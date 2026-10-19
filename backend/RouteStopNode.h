#ifndef ROUTE_STOP_NODE_H
#define ROUTE_STOP_NODE_H

#include "Stop.h"
#include <string>

using namespace std;

// RouteStopNode -> a single stop
class RouteStopNode {
public:
    static const int NO_ETA = -1;                 // Last node: nothing comes next
    static constexpr double NO_DISTANCE = -1.0;   // Database NULL distance

private:
    string stopId;
    string stopName;
    StopType stopType;
    bool isRestStop;
    int sequenceIndex;             
    int etaToNextMinutes;          
    double distanceToNextKm;       
    bool etaIsProvisional;         
    RouteStopNode* next;           // next stop node in route

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

#endif 
