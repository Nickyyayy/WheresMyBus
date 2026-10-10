#ifndef UPDATE_H
#define UPDATE_H

#include "Bus.h"
#include <string>

using namespace std;

// Update encapsulates an action submitted by a conductor affecting a bus
class Update {
private:
    string updateId;
    string busId;
    string newStopId;
    int newSequenceIndex;
    bool newSeatsAvailable;
    BusState newState;
    string timestamp;
    bool tripCompleted;

    // Previous state snapshot captured when update is applied
    bool previousRecorded;
    string previousStopId;
    int previousSequenceIndex;
    bool previousSeatsAvailable;
    BusState previousState;

public:
    Update(string id, string busId, string newStopId, int newSequenceIndex,
           bool newSeatsAvailable, BusState newState, string timestamp, bool tripCompleted = false);

    string getUpdateId() const;
    string getBusId() const;
    string getNewStopId() const;
    int getNewSequenceIndex() const;
    bool getNewSeatsAvailable() const;
    BusState getNewState() const;
    string getTimestamp() const;
    bool getTripCompleted() const;

    void setPrevious(string stopId, int sequenceIndex, bool seatsAvailable, BusState state);
    bool hasPrevious() const;
    string getPreviousStopId() const;
    int getPreviousSequenceIndex() const;
    bool getPreviousSeatsAvailable() const;
    BusState getPreviousState() const;

    static string generateId();
    static string currentTimestamp();
};

#endif // UPDATE_H
