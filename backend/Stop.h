#ifndef STOP_H
#define STOP_H

#include <string>
#include <stdexcept>

using namespace std;

// StopType represents the category of a bus stop in the transit network
enum class StopType {
    COLLEGE,
    STOP,
    REST
};

// Converts StopType enum to string representation
string stopTypeToString(StopType type);

// Parses a string into a StopType enum, throws invalid_argument on unknown text
StopType stopTypeFromString(const string& str);

// Stop represents a physical location in the transit network
class Stop {
private:
    string stopId;
    string stopName;
    StopType stopType;
    bool isRestStop;

public:
    Stop(string id, string name, StopType type, bool isRestStop);

    string getStopId() const;
    string getStopName() const;
    StopType getStopType() const;
    bool getIsRestStop() const;
};

#endif // STOP_H
