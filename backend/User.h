#ifndef USER_H
#define USER_H

#include "Update.h"
#include <string>

using namespace std;

// Base class for all system users demonstrating polymorphism
class User {
protected:
    string userId;
    string name;

public:
    User(string id, string name);
    virtual ~User() {}

    virtual string getRole() const = 0;
    string getUserId() const;
    string getName() const;
};

// Student user with view-only capabilities and a preferred stop
class Student : public User {
private:
    string preferredStopId;

public:
    Student(string id, string name, string preferredStopId);

    string getRole() const override;
    string getPreferredStopId() const;
};

// Conductor user operating a specific bus
class Conductor : public User {
private:
    string assignedBusId;

public:
    Conductor(string id, string name, string assignedBusId);

    string getRole() const override;
    string getAssignedBusId() const;

    Update createUpdate(string newStopId, int newSequenceIndex, bool seatsAvailable,
                        BusState newState, bool tripCompleted = false) const;
};

// Department Head privileged supervisory role
class DepartmentHead : public User {
public:
    DepartmentHead(string id, string name);

    string getRole() const override;
};

#endif // USER_H
