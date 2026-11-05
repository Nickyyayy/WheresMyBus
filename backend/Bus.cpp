#include "Bus.h"

using namespace std;

string busStateToString(BusState state) {
    switch (state) {
        case BusState::NOT_AVAILABLE:
            return "Not available";
        case BusState::IN_SERVICE:
            return "In service";
        case BusState::RESTING:
            return "Resting";
        case BusState::MAINTENANCE:
            return "Maintenance";
        default:
            return "Not available";
    }
}

string stopColorToString(StopColor color) {
    switch (color) {
        case StopColor::GREEN:
            return "GREEN";
        case StopColor::BLUE:
            return "BLUE";
        case StopColor::RED:
            return "RED";
        case StopColor::BLACK:
            return "BLACK";
        case StopColor::PURPLE:
            return "PURPLE";
        case StopColor::BROWN:
            return "BROWN";
        default:
            return "BLACK";
    }
}

string stopColorMeaning(StopColor color) {
    switch (color) {
        case StopColor::GREEN:
            return "Available, seats free";
        case StopColor::BLUE:
            return "Available, bus full";
        case StopColor::RED:
            return "Missed (bus already passed)";
        case StopColor::BROWN:
            return "Resting";
        case StopColor::PURPLE:
            return "Maintenance / malfunction";
        case StopColor::BLACK:
            return "Not available";
        default:
            return "Not available";
    }
}

string journeyDirectionToString(JourneyDirection direction) {
    switch (direction) {
        case JourneyDirection::DROP:
            return "Drop journey";
        case JourneyDirection::PICKUP:
            return "Pickup journey";
        default:
            return "Drop journey";
    }
}

Bus::Bus(string busId, string routeId)
    : busId(move(busId)),
      routeId(move(routeId)),
      currentStopId(""),
      currentSequenceIndex(0),
      state(BusState::NOT_AVAILABLE),
      seatsAvailable(true),
      direction(JourneyDirection::DROP) {}

void Bus::updatePosition(string stopId, int sequenceIndex) {
    currentStopId = move(stopId);
    currentSequenceIndex = sequenceIndex;
}

void Bus::resetPosition() {
    currentStopId = "";
    currentSequenceIndex = 0;
    direction = JourneyDirection::DROP;
}

void Bus::setState(BusState newState) {
    state = newState;
}

void Bus::setSeatsAvailable(bool available) {
    seatsAvailable = available;
}

void Bus::setDirection(JourneyDirection newDirection) {
    direction = newDirection;
}

StopColor Bus::getStatusForStop(int queriedSequenceIndex) const {
    if (state == BusState::MAINTENANCE) {
        return StopColor::PURPLE;
    }
    if (state == BusState::NOT_AVAILABLE) {
        return StopColor::BLACK;
    }
    if (state == BusState::RESTING) {
        return StopColor::BROWN;
    }
    // State is IN_SERVICE
    if (direction == JourneyDirection::DROP) {
        // Outbound Drop Journey: passes 1, 2, 3...

        if (currentSequenceIndex > queriedSequenceIndex) {
            return StopColor::RED;
        }
        return seatsAvailable ? StopColor::GREEN : StopColor::BLUE;
    } else {

        if (currentSequenceIndex < queriedSequenceIndex) {
            return StopColor::RED;
        }
        return seatsAvailable ? StopColor::GREEN : StopColor::BLUE;
    }
}

string Bus::getBusId() const {
    return busId;
}

string Bus::getRouteId() const {
    return routeId;
}

string Bus::getCurrentStopId() const {
    return currentStopId;
}

int Bus::getCurrentSequenceIndex() const {
    return currentSequenceIndex;
}

BusState Bus::getState() const {
    return state;
}

bool Bus::getHasFreeSeats() const {
    return seatsAvailable;
}

JourneyDirection Bus::getDirection() const {
    return direction;
}

bool Bus::isPickupJourney() const {
    return direction == JourneyDirection::PICKUP;
}

// Exactly one operator overload in the entire project
ostream& operator<<(ostream& os, const Bus& bus) {
    os << "Bus " << bus.busId << " | route " << bus.routeId << " | ";
    if (bus.currentSequenceIndex == 0) {
        os << "no position reported yet";
    } else {
        os << "at " << bus.currentStopId << " (position " << bus.currentSequenceIndex << ")";
    }
    os << " | " << busStateToString(bus.state) << " | ";
    os << (bus.seatsAvailable ? "seats free" : "FULL");
    if (bus.state == BusState::IN_SERVICE) {
        os << " (" << (bus.direction == JourneyDirection::DROP ? "Drop" : "Pickup") << ")";
    }
    return os;
}
