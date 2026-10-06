#include "Route.h"
using namespace std;

Route::Route(string id, string name, string description, string startStopId, string restStopId)
    : routeId(move(id)),routeName(move(name)),
      description(move(description)),
      startStopId(move(startStopId)),
      restStopId(move(restStopId)),
      head(nullptr),tail(nullptr),
      totalStops(0) {}

Route::~Route() {
    // save next pointer before deleting each node
    RouteStopNode* current = head;
    while (current != nullptr) {
        RouteStopNode* nextNode = current->getNext();
        delete current;
        current = nextNode;
        }
    head = nullptr;
    tail = nullptr;
    totalStops = 0;
}

bool Route::addStopToEnd(RouteStopNode* node) {
    if(node == nullptr) {
        return false;
    }
    if(node->getNext() != nullptr) {
        return false;
    }
// to verify sequenceIndex is strictly sequential: 1, 2, 3...
    if (node->getSequenceIndex() != totalStops + 1) {
        return false;
    }

    if (head == nullptr) {
        head = node;
        tail = node;
    } else {
        tail->setNext(node);
        tail = node;
    }
    totalStops++;
    return true;
}

RouteStopNode* Route::getHead() const {
    return head;
}

RouteStopNode* Route::getLastStop() const {
    return tail;
}

RouteStopNode* Route::findStopByIndex(int sequenceIndex) const {
    RouteStopNode* current = head;
    while (current != nullptr) {
        if (current->getSequenceIndex() == sequenceIndex) {
            return current;
        }
        current = current->getNext();
    }
    return nullptr;
}

RouteStopNode* Route::findStopById(const string& stopId) const {
    RouteStopNode* current = head;
    while (current != nullptr) {
        if (current->getStopId() == stopId) {
            return current;
        }
        current = current->getNext();
    }
    return nullptr;
}

RouteStopNode* Route::findLastStopById(const string& stopId) const {
    RouteStopNode* lastMatch = nullptr;
    RouteStopNode* current = head;

    while (current != nullptr) {
        if (current->getStopId() == stopId)
            lastMatch = current;

        current = current->getNext();
    }
    return lastMatch;
}

int Route::getTotalStops() const {
    return totalStops;
}

string Route::getRouteId() const {
    return routeId;
}

string Route::getRouteName() const {
    return routeName;
}

string Route::getDescription() const {
    return description;
}

string Route::getStartStopId() const {
    return startStopId;
}

string Route::getRestStopId() const {
    return restStopId;
}

void Route::printRoute() const {

    cout << "Route " << routeId << ": " << routeName << " (" << totalStops << " stops)" << endl;
    RouteStopNode* current = head;

    while (current != nullptr) {

        cout << "  #" << current->getSequenceIndex() << " [" << current->getStopId() << "] "
             << current->getStopName();

        if (current->getIsRestStop())
        {
            cout << " (REST)";
        }
        if (current->hasEtaToNext())
        {
            cout << " -> ETA " << current->getEtaToNextMinutes() << " min";
        }
        cout << endl;
        current = current->getNext();
    }
}
