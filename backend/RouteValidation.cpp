#include "RouteValidation.h"

using namespace std;

bool isValidStopSequence(const Route* route, const vector<string>& claimedStopIds) {
    if (route == nullptr) {
        return false;
    }
    if (route->getTotalStops() != static_cast<int>(claimedStopIds.size())) {
        return false;
    }

    RouteStopNode* current = route->getHead();
    for (size_t i = 0; i < claimedStopIds.size(); i++) {
        if (current == nullptr || current->getStopId() != claimedStopIds[i]) {
            return false;
        }
        current = current->getNext();
    }
    return current == nullptr;
}

bool isValidPosition(const Route* route, const string& stopId, int sequenceIndex) {
    if (route == nullptr || sequenceIndex < 1 || sequenceIndex > route->getTotalStops()) {
        return false;
    }
    RouteStopNode* node = route->findStopByIndex(sequenceIndex);
    return node != nullptr && node->getStopId() == stopId;
}

bool isValidRouteStructure(const Route* route, string& problemOut) {
    if (route == nullptr) {
        problemOut = "Route is null pointer.";
        return false;
    }
    if (route->getTotalStops() < 2) {
        problemOut = "Route must have at least 2 stops.";
        return false;
    }
    if (route->getHead() == nullptr || route->getHead()->getStopId() != route->getStartStopId()) {
        problemOut = "Route start stop (" + route->getStartStopId() + ") does not match head stop.";
        return false;
    }
    if (route->findStopById(route->getRestStopId()) == nullptr) {
        problemOut = "Route does not contain its designated rest stop (" + route->getRestStopId() + ").";
        return false;
    }
    return true;
}
