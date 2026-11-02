#include "Update.h"
#include <ctime>
#include <cstdio>

using namespace std;

Update::Update(string id, string busId, string newStopId, int newSequenceIndex,
               bool newSeatsAvailable, BusState newState, string timestamp, bool tripCompleted,
               JourneyDirection newDirection)
    : updateId(move(id)),
      busId(move(busId)),
      newStopId(move(newStopId)),
      newSequenceIndex(newSequenceIndex),
      newSeatsAvailable(newSeatsAvailable),
      newState(newState),
      newDirection(newDirection),
      timestamp(move(timestamp)),
      tripCompleted(tripCompleted),
      previousRecorded(false),
      previousStopId(""),
      previousSequenceIndex(0),
      previousSeatsAvailable(true),
      previousState(BusState::NOT_AVAILABLE),
      previousDirection(JourneyDirection::DROP) {}

string Update::getUpdateId() const {
    return updateId;
}

string Update::getBusId() const {
    return busId;
}

string Update::getNewStopId() const {
    return newStopId;
}

int Update::getNewSequenceIndex() const {
    return newSequenceIndex;
}

bool Update::getNewSeatsAvailable() const {
    return newSeatsAvailable;
}

BusState Update::getNewState() const {
    return newState;
}

JourneyDirection Update::getNewDirection() const {
    return newDirection;
}

string Update::getTimestamp() const {
    return timestamp;
}

bool Update::getTripCompleted() const {
    return tripCompleted;
}

void Update::setPrevious(string stopId, int sequenceIndex, bool seatsAvailable, BusState state, JourneyDirection dir) {
    previousStopId = move(stopId);
    previousSequenceIndex = sequenceIndex;
    previousSeatsAvailable = seatsAvailable;
    previousState = state;
    previousDirection = dir;
    previousRecorded = true;
}

JourneyDirection Update::getPreviousDirection() const {
    return previousDirection;
}

bool Update::hasPrevious() const {
    return previousRecorded;
}

string Update::getPreviousStopId() const {
    return previousStopId;
}

int Update::getPreviousSequenceIndex() const {
    return previousSequenceIndex;
}

bool Update::getPreviousSeatsAvailable() const {
    return previousSeatsAvailable;
}

BusState Update::getPreviousState() const {
    return previousState;
}

string Update::generateId() {
    static int counter = 0;
    counter++;
    return "UPD" + to_string(counter);
}

string Update::currentTimestamp() {
    time_t now = time(nullptr);
    tm* localTm = localtime(&now);
    char buffer[16];
    if (localTm != nullptr) {
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
                 localTm->tm_hour, localTm->tm_min, localTm->tm_sec);
    } else {
        snprintf(buffer, sizeof(buffer), "00:00:00");
    }
    return string(buffer);
}
