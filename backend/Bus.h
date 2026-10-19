#ifndef BUS_H
#define BUS_H

#include <string>
#include <iostream>

using namespace std;

enum class BusState {
    NOT_AVAILABLE,
    IN_SERVICE,
    RESTING,
    MAINTENANCE
};


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
string stopColorToString(StopColor color);
string stopColorMeaning(StopColor color);


class Bus {
private:
    string busId;
    string routeId;
    string currentStopId;
    int currentSequenceIndex; // 0 = no position reported
    BusState state;
    bool seatsAvailable;

public:
    Bus(string busId, string routeId);

    void updatePosition(string stopId, int sequenceIndex);
    void resetPosition();
    void setState(BusState newState);
    void setSeatsAvailable(bool available);


    // status color of this bus relative to route position?
    StopColor getStatusForStop(int queriedSequenceIndex) const;

    string getBusId() const;
    string getRouteId() const;
    string getCurrentStopId() const;
    int getCurrentSequenceIndex() const;
    BusState getState() const;
    bool getHasFreeSeats() const;

    
    friend ostream& operator<<(ostream& os, const Bus& bus);
};

#endif 
