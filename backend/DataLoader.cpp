#include "DataLoader.h"
#include "RouteValidation.h"
#include <map>
#include <memory>
#include <stdexcept>

using namespace std;

DataLoader::DataLoader(Database& database) : db(database), lastError("") {}

string DataLoader::getLastError() const {
    return lastError;
}

bool DataLoader::loadRoutes(RouteRegistry& routeRegistry) {
    lastError = "";
    try {
        // 1. Load all stops into a local lookup map
        vector<vector<string>> stopRows;
        if (!db.query("SELECT stop_id, stop_name, stop_type, is_rest_stop FROM stops;", stopRows)) {
            lastError = "Failed to query stops: " + db.getLastError();
            return false;
        }
        if (stopRows.empty()) {
            lastError = "stops table is empty";
            return false;
        }

        map<string, Stop> stopsMap;
        for (const auto& row : stopRows) {
            if (row.size() < 4) {
                lastError = "Invalid row format in stops table";
                return false;
            }
            string id = row[0];
            string name = row[1];
            StopType type = stopTypeFromString(row[2]);
            bool isRest = (row[3] == "1");
            stopsMap.emplace(id, Stop(id, name, type, isRest));
        }

        // 2. Load all routes
        vector<vector<string>> routeRows;
        if (!db.query("SELECT route_id, route_name, COALESCE(description,''), start_stop_id, rest_stop_id FROM routes ORDER BY route_id;", routeRows)) {
            lastError = "Failed to query routes: " + db.getLastError();
            return false;
        }
        if (routeRows.empty()) {
            lastError = "routes table is empty";
            return false;
        }

        // 3. For each route, build Route and load its ordered stops with ETAs
        for (const auto& rRow : routeRows) {
            if (rRow.size() < 5) {
                lastError = "Invalid row format in routes table";
                return false;
            }
            string routeId = rRow[0];
            string routeName = rRow[1];
            string description = rRow[2];
            string startStopId = rRow[3];
            string restStopId = rRow[4];

            unique_ptr<Route> route = make_unique<Route>(routeId, routeName, description, startStopId, restStopId);

            // Execute the exact join query
            string sql = "SELECT rs.stop_order, rs.stop_id, "
                         "COALESCE(e.estimated_minutes, -1), COALESCE(e.distance_km, -1), COALESCE(e.is_provisional, 1) "
                         "FROM route_stops rs "
                         "LEFT JOIN route_stops nxt ON nxt.route_id = rs.route_id AND nxt.stop_order = rs.stop_order + 1 "
                         "LEFT JOIN etas e ON e.route_id = rs.route_id AND e.from_stop_id = rs.stop_id "
                         "AND e.to_stop_id = nxt.stop_id "
                         "WHERE rs.route_id = '" + db.escape(routeId) + "' "
                         "ORDER BY rs.stop_order;";

            vector<vector<string>> rsRows;
            if (!db.query(sql, rsRows)) {
                lastError = "Failed to query route_stops for route " + routeId + ": " + db.getLastError();
                return false;
            }
            if (rsRows.empty()) {
                lastError = "route " + routeId + ": has no stops in route_stops";
                return false;
            }

            vector<string> loadedStopIds;
            for (size_t i = 0; i < rsRows.size(); i++) {
                const auto& rsRow = rsRows[i];
                if (rsRow.size() < 5) {
                    lastError = "Invalid route_stop row on route " + routeId;
                    return false;
                }

                int stopOrder = stoi(rsRow[0]);
                string stopId = rsRow[1];
                int eta = stoi(rsRow[2]);
                double dist = stod(rsRow[3]);
                bool isProvisional = (rsRow[4] != "0");

                auto stopIt = stopsMap.find(stopId);
                if (stopIt == stopsMap.end()) {
                    lastError = "route " + routeId + ": unknown stop id " + stopId;
                    return false;
                }
                const Stop& stop = stopIt->second;

                bool isLastRow = (i == rsRows.size() - 1);
                if (isLastRow) {
                    eta = RouteStopNode::NO_ETA;
                    dist = RouteStopNode::NO_DISTANCE;
                } else {
                    if (eta < 0) {
                        lastError = "route " + routeId + ": no ETA stored from stop " + stopId +
                                    " (position " + to_string(stopOrder) + ") to the next stop";
                        return false;
                    }
                    if (dist < 0.0) {
                        dist = RouteStopNode::NO_DISTANCE;
                    }
                }

                RouteStopNode* node = new RouteStopNode(stop, stopOrder, eta, dist, isProvisional);
                if (!route->addStopToEnd(node)) {
                    delete node;
                    lastError = "route " + routeId + ": stop_order values are not 1,2,3... without gaps";
                    return false;
                }
                loadedStopIds.push_back(stopId);
            }

            // 5. Route structure and sequence validation
            string problem;
            if (!isValidRouteStructure(route.get(), problem)) {
                lastError = "route " + routeId + " structure invalid: " + problem;
                return false;
            }
            if (!isValidStopSequence(route.get(), loadedStopIds)) {
                lastError = "route " + routeId + " stop sequence invalid.";
                return false;
            }

            if (!routeRegistry.addRoute(route.get())) {
                lastError = "failed to add route " + routeId + " to registry.";
                return false;
            }
            route.release(); // Ownership successfully transferred to registry
        }

        return true;
    } catch (const exception& e) {
        lastError = "bad data while loading routes: " + string(e.what());
        return false;
    }
}

bool DataLoader::loadBuses(BusRegistry& busRegistry, const RouteRegistry& routeRegistry) {
    lastError = "";
    try {
        vector<vector<string>> busRows;
        if (!db.query("SELECT bus_number, route_id FROM buses ORDER BY route_id, CAST(bus_number AS UNSIGNED);", busRows)) {
            lastError = "Failed to query buses: " + db.getLastError();
            return false;
        }
        if (busRows.empty()) {
            lastError = "buses table is empty";
            return false;
        }

        for (const auto& row : busRows) {
            if (row.size() < 2) {
                lastError = "Invalid row format in buses table";
                return false;
            }
            string busNumber = row[0];
            string routeId = row[1];

            if (routeRegistry.getRouteById(routeId) == nullptr) {
                lastError = "bus " + busNumber + " assigned to unknown route " + routeId;
                return false;
            }

            Bus* bus = new Bus(busNumber, routeId);
            if (!busRegistry.addBus(bus)) {
                delete bus;
                lastError = "duplicate bus number " + busNumber;
                return false;
            }
        }

        return true;
    } catch (const exception& e) {
        lastError = "bad data while loading buses: " + string(e.what());
        return false;
    }
}
