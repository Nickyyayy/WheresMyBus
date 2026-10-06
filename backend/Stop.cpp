#include "Stop.h"
using namespace std;

string stopTypeToString(StopType type)
{
    switch (type) {
        case StopType::COLLEGE:
            return "COLLEGE";

        case StopType::STOP:
            return "STOP";

        case StopType::REST:
            return "REST";

        default:
            return "STOP";
    }
}

StopType stopTypeFromString(const string& str) {
    if(str == "COLLEGE")
        return StopType::COLLEGE;

    else if (str == "STOP")
        return StopType::STOP;

    else if (str == "REST")
        return StopType::REST;

    throw invalid_argument("Unknown stop type: " + str);
}

Stop::Stop(string id, string name, StopType type, bool isRestStop) : stopId(move(id)), stopName(move(name)), stopType(type), isRestStop(isRestStop) {}

string Stop::getStopId() const {
    return stopId;
}

string Stop::getStopName() const {
    return stopName;
}

StopType Stop::getStopType() const {
    return stopType;
}

bool Stop::getIsRestStop() const {
    return isRestStop;
}
