#ifndef STOP_H
#define STOP_H

#include <string>
#include <stdexcept>

using namespace std;

// StopType = category of a bus stop 
enum class StopType {
    COLLEGE,
    STOP,
    REST
};

// Converts StopType enum -> string representation
string stopTypeToString(StopType type);

StopType stopTypeFromString(const string& str);


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

#endif 
