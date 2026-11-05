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
    RouteStopNode* node = nullptr;
    if (bus.getDirection() == JourneyDirection::PICKUP) {
        // On inbound pickup journey heading towards start stop (Graphic Era, pos 1)
        node = route->findStopById(stopId);
    } else {
        // On outbound drop journey, using the last occurrence handles start/end loops
        node = route->findLastStopById(stopId);
    }
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
                       bus->getHasFreeSeats(), bus->getState(), bus->getDirection());

    // Apply new values to bus
    if (update.getNewSequenceIndex() == 0) {
        bus->resetPosition();
    } else {
        bus->updatePosition(update.getNewStopId(), update.getNewSequenceIndex());
    }
    bus->setState(update.getNewState());
    bus->setSeatsAvailable(update.getNewSeatsAvailable());
    bus->setDirection(update.getNewDirection());

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
    Update u = conductor.createUpdate(head->getStopId(), 1, true, BusState::IN_SERVICE, false, JourneyDirection::DROP);
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

    RouteStopNode* restNode = route->findRestStop();
    if (restNode == nullptr) {
        restNode = route->findStopById(route->getRestStopId());
    }
    if (restNode == nullptr) {
        return {false, "Rest stop not found on route."};
    }

    Update u = conductor.createUpdate(restNode->getStopId(), restNode->getSequenceIndex(),
                                      true, BusState::RESTING, false, JourneyDirection::PICKUP);
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

    RouteStopNode* currNode = route->findStopByIndex(bus->getCurrentSequenceIndex());
    if (currNode == nullptr) {
        return {false, "Current stop not found on route."};
    }

    // CASE 1: Bus is currently RESTING at the Rest Stop
    // Moving next transitions from rest to the PICKUP journey heading back towards Graphic Era
    if (bus->getState() == BusState::RESTING) {
        RouteStopNode* nextNode = currNode->getPrev();
        if (nextNode != nullptr) {
            Update u = conductor.createUpdate(nextNode->getStopId(), nextNode->getSequenceIndex(),
                                              bus->getHasFreeSeats(), BusState::IN_SERVICE, false, JourneyDirection::PICKUP);
            ActionResult res = submit(u);
            if (!res.ok) {
                return res;
            }
            return {true, "Reached " + nextNode->getStopName() + " (position " +
                           to_string(nextNode->getSequenceIndex()) + ")."};
        } else {
            // Rest stop was head: wrap to college
            RouteStopNode* headNode = route->getHead();
            Update u = conductor.createUpdate(headNode->getStopId(), 1,
                                              bus->getHasFreeSeats(), BusState::IN_SERVICE, true, JourneyDirection::DROP);
            ActionResult res = submit(u);
            if (!res.ok) return res;
            return {true, "Reached " + headNode->getStopName() + ". Trip complete! Bus is back at the start, ready for the next trip."};
        }
    }

    // CASE 2: Bus is on OUTBOUND DROP JOURNEY (traveling forward via next pointer)
    if (bus->getDirection() == JourneyDirection::DROP) {
        RouteStopNode* nextNode = currNode->getNext();
        if (nextNode == nullptr) {
            return {false, "No next stop available on route."};
        }

        bool isRest = (nextNode->getIsRestStop() || nextNode->getStopId() == route->getRestStopId());
        if (isRest) {
            // Reached Rest Stop: Drop journey complete! Bus is now resting, ready for subsequent pickup journey
            Update u = conductor.createUpdate(nextNode->getStopId(), nextNode->getSequenceIndex(),
                                              bus->getHasFreeSeats(), BusState::RESTING, false, JourneyDirection::PICKUP);
            ActionResult res = submit(u);
            if (!res.ok) {
                return res;
            }
            return {true, "Reached " + nextNode->getStopName() + " (position " +
                           to_string(nextNode->getSequenceIndex()) + "). Drop journey complete! Bus is now resting."};
        } else {
            // Intermediate stop along drop journey
            Update u = conductor.createUpdate(nextNode->getStopId(), nextNode->getSequenceIndex(),
                                              bus->getHasFreeSeats(), BusState::IN_SERVICE, false, JourneyDirection::DROP);
            ActionResult res = submit(u);
            if (!res.ok) {
                return res;
            }
            return {true, "Reached " + nextNode->getStopName() + " (position " +
                           to_string(nextNode->getSequenceIndex()) + ")."};
        }
    }

    // CASE 3: Bus is on INBOUND PICKUP JOURNEY (traveling backward via prev pointer towards Graphic Era)
    if (bus->getDirection() == JourneyDirection::PICKUP) {
        RouteStopNode* nextNode = currNode->getPrev();
        if (nextNode == nullptr || nextNode->getStopId() == route->getStartStopId() || nextNode == route->getHead()) {
            // Reached Graphic Era: Pickup journey complete!
            RouteStopNode* headNode = route->getHead();
            Update u = conductor.createUpdate(headNode->getStopId(), 1,
                                              bus->getHasFreeSeats(), BusState::IN_SERVICE, true, JourneyDirection::DROP);
            ActionResult res = submit(u);
            if (!res.ok) {
                return res;
            }
            return {true, "Reached " + headNode->getStopName() +
                           ". Trip complete! Bus is back at the start, ready for the next trip."};
        } else {
            // Intermediate stop along pickup journey
            Update u = conductor.createUpdate(nextNode->getStopId(), nextNode->getSequenceIndex(),
                                              bus->getHasFreeSeats(), BusState::IN_SERVICE, false, JourneyDirection::PICKUP);
            ActionResult res = submit(u);
            if (!res.ok) {
                return res;
            }
            return {true, "Reached " + nextNode->getStopName() + " (position " +
                           to_string(nextNode->getSequenceIndex()) + ")."};
        }
    }

    return {false, "Unknown journey state."};
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
                                      seatsFree, bus->getState(), false, bus->getDirection());
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
                                          bus->getHasFreeSeats(), BusState::MAINTENANCE, false, bus->getDirection());
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
                                          bus->getHasFreeSeats(), restoredState, false, bus->getDirection());
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

    Update u = conductor.createUpdate("", 0, true, BusState::NOT_AVAILABLE, false, JourneyDirection::DROP);
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
    bus->setDirection(undone.getPreviousDirection());

    return {true, "Undid the update from " + undone.getTimestamp() + "."};
}
