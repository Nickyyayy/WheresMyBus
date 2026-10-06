#include "RouteStopNode.h"
using namespace std;

RouteStopNode::RouteStopNode(const Stop& stop, int sequenceIndex, int etaToNextMinutes,
                             double distanceToNextKm, bool etaIsProvisional)
    : stopId(stop.getStopId()),
      stopName(stop.getStopName()),
      stopType(stop.getStopType()),
      isRestStop(stop.getIsRestStop()),
      sequenceIndex(sequenceIndex),
      etaToNextMinutes(etaToNextMinutes),
      distanceToNextKm(distanceToNextKm),
      etaIsProvisional(etaIsProvisional),
      next(nullptr) {}

string RouteStopNode::getStopId() const {
    return stopId;
}

string RouteStopNode::getStopName() const {
    return stopName;
}

StopType RouteStopNode::getStopType() const {
    return stopType;
}

bool RouteStopNode::getIsRestStop() const {
    return isRestStop;
}

int RouteStopNode::getSequenceIndex() const {
    return sequenceIndex;
}

bool RouteStopNode::hasEtaToNext() const {
    return etaToNextMinutes != NO_ETA;
}

int RouteStopNode::getEtaToNextMinutes() const {
    return etaToNextMinutes;
}

bool RouteStopNode::hasDistanceToNext() const {
    return distanceToNextKm >= 0.0;
}

double RouteStopNode::getDistanceToNextKm() const {
    return distanceToNextKm;
}

bool RouteStopNode::getEtaIsProvisional() const {
    return etaIsProvisional;
}

RouteStopNode* RouteStopNode::getNext() const {
    return next;
}

void RouteStopNode::setNext(RouteStopNode* nextNode) {
    next = nextNode;
}
