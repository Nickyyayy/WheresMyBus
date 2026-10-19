#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include "Database.h"
#include "RouteRegistry.h"
#include "BusRegistry.h"
#include <string>

using namespace std;

// DataLoader fills domain registries with sql csv data
class DataLoader {
private:
    Database& db;
    string lastError;

public:
    explicit DataLoader(Database& database);

    bool loadRoutes(RouteRegistry& routeRegistry);
    bool loadBuses(BusRegistry& busRegistry, const RouteRegistry& routeRegistry);
    string getLastError() const;
};

#endif 
