#include "User.h"

using namespace std;

// Base User
User::User(string id, string name) : userId(move(id)), name(move(name)) {}

string User::getUserId() const {
    return userId;
}

string User::getName() const {
    return name;
}

// Student
Student::Student(string id, string name, string preferredStopId)
    : User(move(id), move(name)), preferredStopId(move(preferredStopId)) {}

string Student::getRole() const {
    return "Student";
}

string Student::getPreferredStopId() const {
    return preferredStopId;
}

// Conductor
Conductor::Conductor(string id, string name, string assignedBusId)
    : User(move(id), move(name)), assignedBusId(move(assignedBusId)) {}

string Conductor::getRole() const {
    return "Conductor";
}

string Conductor::getAssignedBusId() const {
    return assignedBusId;
}

Update Conductor::createUpdate(string newStopId, int newSequenceIndex, bool seatsAvailable,
                               BusState newState, bool tripCompleted) const {
    return Update(Update::generateId(), assignedBusId, move(newStopId), newSequenceIndex,
                  seatsAvailable, newState, Update::currentTimestamp(), tripCompleted);
}

// DepartmentHead
DepartmentHead::DepartmentHead(string id, string name) : User(move(id), move(name)) {}

string DepartmentHead::getRole() const {
    return "Department Head";
}
