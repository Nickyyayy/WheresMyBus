#ifndef BUS_STATUS_ENGINE_H
#define BUS_STATUS_ENGINE_H

#include "RouteRegistry.h"
#include "BusRegistry.h"
#include "Queue.h"
#include "Stack.h"
#include "Update.h"
#include "User.h"
#include "Bus.h"
#include <string>
#include <vector>

using namespace std;

// Result structure returned by conductor actions
struct ActionResult {
    bool ok;
    string message;
};

// BusStatusEngine coordinates domain state transitions, update queueing, and history tracking
class BusStatusEngine {
private:
    RouteRegistry& routeRegistry;
    BusRegistry& busRegistry;
    Queue<Update>& pendingUpdates;
    Stack<Update>& updateHistory;

    ActionResult submit(const Update& update);
    ActionResult applyUpdate(Update& update);

public:
    BusStatusEngine(RouteRegistry& routes, BusRegistry& buses,
                    Queue<Update>& pending, Stack<Update>& history);

    ActionResult startShiftAtCollege(const Conductor& conductor);
    ActionResult startShiftAtRestStop(const Conductor& conductor);
    ActionResult reachedNextStop(const Conductor& conductor);
    ActionResult setSeatsAvailable(const Conductor& conductor, bool seatsFree);
    ActionResult setMaintenance(const Conductor& conductor, bool underMaintenance);
    ActionResult goOffDuty(const Conductor& conductor);
    ActionResult undoLastUpdate(const Conductor& conductor);

    StopColor getStatusForBusAtStop(const Bus& bus, const string& stopId) const;
    string getCurrentStopName(const Bus& bus) const;
    const RouteRegistry& getRouteRegistry() const;
    const BusRegistry& getBusRegistry() const;
    vector<Update> getHistory() const;
};

#endif 
