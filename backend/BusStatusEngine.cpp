#include "BusStatusEngine.h"
#include "RouteValidation.h"

using namespace std;

BusStatusEngine::BusStatusEngine(RouteRegistry& routes, BusRegistry& buses,
                                 Queue<Update>& pending, Stack<Update>& history)
    : routeRegistry(routes),
      busRegistry(buses),
      pendingUpdates(pending),
      updateHistory(history) {}

const RouteRegistry& BusStatusEngine::getRouteRegistry() const {
    return routeRegistry;
}

const BusRegistry& BusStatusEngine::getBusRegistry() const {
    return busRegistry;
}

vector<Update> BusStatusEngine::getHistory() const {
    return updateHistory.toVector();
}

string BusStatusEngine::getCurrentStopName(const Bus& bus) const {
    if (bus.getCurrentSequenceIndex() == 0) {
        return "";
    }
    Route* route = routeRegistry.getRouteById(bus.getRouteId());
    if (route == nullptr) {
        return "";
    }
    RouteStopNode* node = route->findStopByIndex(bus.getCurrentSequenceIndex());
    return node != nullptr ? node->getStopName() : "";
}

StopColor BusStatusEngine::getStatusForBusAtStop(const Bus& bus, const string& stopId) const {
    Route* route = routeRegistry.getRouteById(bus.getRouteId());
    if (route == nullptr) {
        return StopColor::BLACK;
    }
    // Using the LAST occurrence ensures return stops (e.g. Graphic Era) aren't prematurely marked 'missed'
    RouteStopNode* node = route->findLastStopById(stopId);
    if (node == nullptr) {
        return StopColor::BLACK;
    }
    return bus.getStatusForStop(node->getSequenceIndex());
}

ActionResult BusStatusEngine::applyUpdate(Update& update) {
    Bus* bus = busRegistry.getBusById(update.getBusId());
    if (bus == nullptr) {
        return {false, "Unknown bus " + update.getBusId()};
    }

    Route* route = routeRegistry.getRouteById(bus->getRouteId());
    if (route == nullptr) {
        return {false, "Rejected update: route " + bus->getRouteId() + " not found"};
    }

    // Validation rules
    if (update.getNewSequenceIndex() == 0) {
        if (update.getNewState() != BusState::NOT_AVAILABLE && update.getNewState() != BusState::MAINTENANCE) {
            return {false, "Rejected update: position 0 requires NOT_AVAILABLE or MAINTENANCE state"};
        }
        if (!update.getNewStopId().empty()) {
            return {false, "Rejected update: position 0 requires empty stop id"};
        }
    } else {
        if (!isValidPosition(route, update.getNewStopId(), update.getNewSequenceIndex())) {
            return {false, "Rejected update: invalid position on route"};
        }
    }

    // Capture bus's previous state
    update.setPrevious(bus->getCurrentStopId(), bus->getCurrentSequenceIndex(),
                       bus->getHasFreeSeats(), bus->getState());

    // Apply new values to bus
    if (update.getNewSequenceIndex() == 0) {
        bus->resetPosition();
    } else {
        bus->updatePosition(update.getNewStopId(), update.getNewSequenceIndex());
    }
    bus->setState(update.getNewState());
    bus->setSeatsAvailable(update.getNewSeatsAvailable());

    // Record into history stack
    updateHistory.push(update);

    return {true, ""};
}

ActionResult BusStatusEngine::submit(const Update& update) {
    pendingUpdates.push(update);
    ActionResult result{false, ""};
    while (!pendingUpdates.isEmpty()) {
        Update u = pendingUpdates.pop();
        result = applyUpdate(u);
    }
    return result;
}

ActionResult BusStatusEngine::startShiftAtCollege(const Conductor& conductor) {
    Bus* bus = busRegistry.getBusById(conductor.getAssignedBusId());
    if (bus == nullptr) {
        return {false, "Unknown bus " + conductor.getAssignedBusId()};
    }
    if (bus->getState() != BusState::NOT_AVAILABLE) {
        return {false, "Shift already started. Use 'Reached next stop'."};
    }

    Route* route = routeRegistry.getRouteById(bus->getRouteId());
    if (route == nullptr || route->getHead() == nullptr) {
        return {false, "Route not found or has no stops."};
    }

    RouteStopNode* head = route->getHead();
    Update u = conductor.createUpdate(head->getStopId(), 1, true, BusState::IN_SERVICE, false);
    ActionResult res = submit(u);
    if (!res.ok) {
        return res;
    }
    return {true, "Shift started at " + head->getStopName() + " (position 1). Bus is in service."};
}

ActionResult BusStatusEngine::startShiftAtRestStop(const Conductor& conductor) {
    Bus* bus = busRegistry.getBusById(conductor.getAssignedBusId());
    if (bus == nullptr) {
        return {false, "Unknown bus " + conductor.getAssignedBusId()};
    }
    if (bus->getState() != BusState::NOT_AVAILABLE) {
        return {false, "Shift already started. Use 'Reached next stop'."};
    }

    Route* route = routeRegistry.getRouteById(bus->getRouteId());
    if (route == nullptr) {
        return {false, "Route not found."};
    }

    RouteStopNode* restNode = route->findStopById(route->getRestStopId());
    if (restNode == nullptr) {
        return {false, "Rest stop not found on route."};
    }

    Update u = conductor.createUpdate(restNode->getStopId(), restNode->getSequenceIndex(),
                                      true, BusState::RESTING, false);
    ActionResult res = submit(u);
    if (!res.ok) {
        return res;
    }
    return {true, "Shift started at the rest stop: " + restNode->getStopName() +
                   " (position " + to_string(restNode->getSequenceIndex()) + "). Bus is resting."};
}

ActionResult BusStatusEngine::reachedNextStop(const Conductor& conductor) {
    Bus* bus = busRegistry.getBusById(conductor.getAssignedBusId());
    if (bus == nullptr) {
        return {false, "Unknown bus " + conductor.getAssignedBusId()};
    }
    if (bus->getState() == BusState::MAINTENANCE) {
        return {false, "Bus is under maintenance. Mark it fixed first."};
    }
    if (bus->getState() == BusState::NOT_AVAILABLE || bus->getCurrentSequenceIndex() == 0) {
        return {false, "Start your shift first (0 = Graphic Era, 1 = rest stop)."};
    }

    Route* route = routeRegistry.getRouteById(bus->getRouteId());
    if (route == nullptr) {
        return {false, "Route not found."};
    }

    int nextIndex = bus->getCurrentSequenceIndex() + 1;
    RouteStopNode* nextNode = route->findStopByIndex(nextIndex);
    if (nextNode == nullptr) {
        return {false, "No next stop available on route."};
    }

    // Check if next stop is the final position of the route
    if (nextNode->getNext() == nullptr) {
        // Trip is complete: bus returns to position 1 (head of route), state IN_SERVICE
        RouteStopNode* headNode = route->getHead();
        Update u = conductor.createUpdate(headNode->getStopId(), 1,
                                          bus->getHasFreeSeats(), BusState::IN_SERVICE, true);
        ActionResult res = submit(u);
        if (!res.ok) {
            return res;
        }
        return {true, "Reached " + nextNode->getStopName() +
                       ". Trip complete! Bus is back at the start, ready for the next trip."};
    }

    // Next stop is an intermediate stop
    bool isRest = (nextNode->getIsRestStop() || nextNode->getStopId() == route->getRestStopId());
    BusState newState = isRest ? BusState::RESTING : BusState::IN_SERVICE;

    Update u = conductor.createUpdate(nextNode->getStopId(), nextIndex,
                                      bus->getHasFreeSeats(), newState, false);
    ActionResult res = submit(u);
    if (!res.ok) {
        return res;
    }

    if (isRest) {
        return {true, "Reached " + nextNode->getStopName() + " (position " +
                       to_string(nextIndex) + "). Bus is now resting."};
    } else {
        return {true, "Reached " + nextNode->getStopName() + " (position " +
                       to_string(nextIndex) + ")."};
    }
}

ActionResult BusStatusEngine::setSeatsAvailable(const Conductor& conductor, bool seatsFree) {
    Bus* bus = busRegistry.getBusById(conductor.getAssignedBusId());
    if (bus == nullptr) {
        return {false, "Unknown bus " + conductor.getAssignedBusId()};
    }
    if (bus->getState() == BusState::MAINTENANCE) {
        return {false, "Bus is under maintenance. Mark it fixed first."};
    }
    if (bus->getState() != BusState::IN_SERVICE) {
        return {false, "Seats can only be changed while the bus is in service."};
    }

    Update u = conductor.createUpdate(bus->getCurrentStopId(), bus->getCurrentSequenceIndex(),
                                      seatsFree, bus->getState(), false);
    ActionResult res = submit(u);
    if (!res.ok) {
        return res;
    }
    return {true, seatsFree ? "Seats marked FREE." : "Bus marked FULL."};
}

ActionResult BusStatusEngine::setMaintenance(const Conductor& conductor, bool underMaintenance) {
    Bus* bus = busRegistry.getBusById(conductor.getAssignedBusId());
    if (bus == nullptr) {
        return {false, "Unknown bus " + conductor.getAssignedBusId()};
    }

    if (underMaintenance) {
        if (bus->getState() == BusState::MAINTENANCE) {
            return {false, "Bus is already under maintenance."};
        }
        Update u = conductor.createUpdate(bus->getCurrentStopId(), bus->getCurrentSequenceIndex(),
                                          bus->getHasFreeSeats(), BusState::MAINTENANCE, false);
        ActionResult res = submit(u);
        if (!res.ok) {
            return res;
        }
        return {true, "Bus marked as under MAINTENANCE."};
    } else {
        if (bus->getState() != BusState::MAINTENANCE) {
            return {false, "Bus is not under maintenance."};
        }

        BusState restoredState;
        if (bus->getCurrentSequenceIndex() == 0) {
            restoredState = BusState::NOT_AVAILABLE;
        } else {
            Route* route = routeRegistry.getRouteById(bus->getRouteId());
            RouteStopNode* node = (route != nullptr) ? route->findStopByIndex(bus->getCurrentSequenceIndex()) : nullptr;
            bool isRest = (node != nullptr) && (node->getIsRestStop() || node->getStopId() == route->getRestStopId());
            restoredState = isRest ? BusState::RESTING : BusState::IN_SERVICE;
        }

        Update u = conductor.createUpdate(bus->getCurrentStopId(), bus->getCurrentSequenceIndex(),
                                          bus->getHasFreeSeats(), restoredState, false);
        ActionResult res = submit(u);
        if (!res.ok) {
            return res;
        }
        return {true, "Maintenance finished. Bus is " + busStateToString(restoredState) + "."};
    }
}

ActionResult BusStatusEngine::goOffDuty(const Conductor& conductor) {
    Bus* bus = busRegistry.getBusById(conductor.getAssignedBusId());
    if (bus == nullptr) {
        return {false, "Unknown bus " + conductor.getAssignedBusId()};
    }

    Update u = conductor.createUpdate("", 0, true, BusState::NOT_AVAILABLE, false);
    ActionResult res = submit(u);
    if (!res.ok) {
        return res;
    }
    return {true, "Shift ended. Bus is now not available."};
}

ActionResult BusStatusEngine::undoLastUpdate(const Conductor& conductor) {
    string targetBusId = conductor.getAssignedBusId();
    Bus* bus = busRegistry.getBusById(targetBusId);
    if (bus == nullptr) {
        return {false, "Unknown bus " + targetBusId};
    }

    vector<Update> all = updateHistory.toVector();
    int k = -1;
    for (int i = 0; i < static_cast<int>(all.size()); i++) {
        if (all[i].getBusId() == targetBusId) {
            k = i;
            break;
        }
    }

    if (k == -1) {
        return {false, "Nothing to undo for bus " + targetBusId + "."};
    }

    // Set aside k updates from other buses
    vector<Update> setAside;
    setAside.reserve(k);
    for (int i = 0; i < k; i++) {
        setAside.push_back(updateHistory.pop());
    }

    // Pop the target update to undo
    Update undone = updateHistory.pop();

    // Push setAside updates back in original order (newest first on stack means push from bottom to top)
    for (int i = k - 1; i >= 0; i--) {
        updateHistory.push(setAside[i]);
    }

    // Restore the bus's previous state
    if (undone.getPreviousSequenceIndex() == 0) {
        bus->resetPosition();
    } else {
        bus->updatePosition(undone.getPreviousStopId(), undone.getPreviousSequenceIndex());
    }
    bus->setSeatsAvailable(undone.getPreviousSeatsAvailable());
    bus->setState(undone.getPreviousState());

    return {true, "Undid the update from " + undone.getTimestamp() + "."};
}
