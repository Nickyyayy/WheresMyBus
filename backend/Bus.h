#ifndef BUS_H
#define BUS_H

#include <string>
#include <iostream>

using namespace std;

// Bus operational states
enum class BusState {
    NOT_AVAILABLE,
    IN_SERVICE,
    RESTING,
    MAINTENANCE
};

// Journey direction: Drop (outbound from college) vs Pickup (inbound to college)
enum class JourneyDirection {
    DROP,   // Graphic Era -> Rest Stop (outbound drop journey)
    PICKUP  // Rest Stop -> Graphic Era (inbound pickup journey)
};

// Colors indicating bus status relative to a specific stop
enum class StopColor {
    GREEN,
    BLUE,
    RED,
    BLACK,
    PURPLE,
    BROWN
};

// String conversions for enum values
string busStateToString(BusState state);
string journeyDirectionToString(JourneyDirection direction);
string stopColorToString(StopColor color);
string stopColorMeaning(StopColor color);

// Bus represents a vehicle assigned to a route
class Bus {
private:
    string busId;
    string routeId;
    string currentStopId;
    int currentSequenceIndex; // 0 means no position reported yet
    BusState state;
    bool seatsAvailable;
    JourneyDirection direction;

public:
    Bus(string busId, string routeId);

    void updatePosition(string stopId, int sequenceIndex);
    void resetPosition();
    void setState(BusState newState);
    void setSeatsAvailable(bool available);
    void setDirection(JourneyDirection newDirection);

    // Calculates the status color of this bus relative to a queried route position
    StopColor getStatusForStop(int queriedSequenceIndex) const;

    string getBusId() const;
    string getRouteId() const;
    string getCurrentStopId() const;
    int getCurrentSequenceIndex() const;
    BusState getState() const;
    bool getHasFreeSeats() const;
    JourneyDirection getDirection() const;
    bool isPickupJourney() const;

    // THE project's only operator overload
    friend ostream& operator<<(ostream& os, const Bus& bus);
};

#endif // BUS_H
